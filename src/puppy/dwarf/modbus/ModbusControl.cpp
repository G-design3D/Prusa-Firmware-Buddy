#include "ModbusControl.hpp"
#include "ModbusRegisters.hpp"
#include <cstring>
#include "log.h"
#include "Marlin/src/module/temperature.h"
#include "Marlin/src/module/stepper/trinamic.h"
#include "../loadcell.hpp"
#include "Pin.hpp"
#include "Cheese.hpp"
#include "tool_filament_sensor.hpp"
#include "timing.h"
#include "ModbusFIFOHandlers.hpp"
#include "fanctl.hpp"
#include "utility_extensions.hpp"
#include "accelerometer.hpp"

namespace dwarf::ModbusControl {

LOG_COMPONENT_DEF(ModbusControl, LOG_SEVERITY_INFO);

struct ModbusMessage {
    uint16_t m_Address;
    uint32_t m_Value;
};

/// Struct used to decompose the 16 bit holding register into 2 8-bit values
union __attribute__((packed)) LedPwm {
    uint16_t reg_value;       ///< 16 bit register value
    struct {
        uint8_t not_selected; ///< 8 LSb PWM when not selected [0 - 0xff]
        uint8_t selected;     ///< 8 MSb PWM when selected [0 - 0xff]
    };

    LedPwm(uint8_t selected_, uint8_t not_selected_)
        : not_selected(not_selected_)
        , selected(selected_) {}
    LedPwm(uint16_t reg_value_)
        : reg_value(reg_value_) {}
    operator uint16_t() const { return reg_value; }
};

static constexpr unsigned int MODBUS_QUEUE_MESSAGE_COUNT = 40;

osMailQDef(m_ModbusQueue, MODBUS_QUEUE_MESSAGE_COUNT, ModbusMessage);
osMailQId m_ModbusQueueHandle;

bool s_isDwarfSelected = false;

void OnReadInputRegister(uint16_t address);
void OnWriteCoil(uint16_t address, bool value);
void OnWriteRegister(uint16_t address, uint16_t value);
bool OnReadFIFO(uint16_t address, uint32_t *pValueCount, std::array<uint16_t, MODBUS_FIFO_MAX_COUNT> &fifo);

void OnTmcReadRequest(uint16_t value);
void OnTmcWriteRequest();
void SetDwarfSelected(bool selected);

bool Init() {
    modbus::ModbusProtocol::SetOnReadInputRegisterCallback(OnReadInputRegister);
    modbus::ModbusProtocol::SetOnWriteCoilCallback(OnWriteCoil);
    modbus::ModbusProtocol::SetOnWriteRegisterCallback(OnWriteRegister);
    modbus::ModbusProtocol::SetOnReadFIFOCallback(OnReadFIFO);

    ModbusRegisters::SetRegValue(ModbusRegisters::SystemHoldingRegister::led_pwm, LedPwm(0, 0)); // LED off

    m_ModbusQueueHandle = osMailCreate(osMailQ(m_ModbusQueue), NULL);
    if (m_ModbusQueueHandle == nullptr) {
        return false;
    }
    SetDwarfSelected(false);

    return true;
}

bool isDwarfSelected() {
    return s_isDwarfSelected;
}

void OnReadInputRegister(uint16_t address) {
    // WARNING: this method is called from different thread

    if (address == static_cast<uint16_t>(ModbusRegisters::SystemInputRegister::time_sync_lo)) {
        // Update cached register value when first part of timer is read
        const uint32_t timestamp_us = ticks_us();
        ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::time_sync_lo, (uint16_t)(timestamp_us & 0x0000ffff));
        ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::time_sync_hi, (uint16_t)(timestamp_us >> 16));
    }
}

void OnWriteCoil(uint16_t address, bool value) {
    // WARNING: this method is called from different thread

    ModbusMessage *msg = (ModbusMessage *)osMailAlloc(m_ModbusQueueHandle, osWaitForever);
    msg->m_Address = address;
    msg->m_Value = value;
    osMailPut(m_ModbusQueueHandle, msg);
}

void OnWriteRegister(uint16_t address, uint16_t value) {
    // WARNING: this method is called from different thread

    if (address == (uint16_t)ModbusRegisters::SystemHoldingRegister::tmc_read_request) {
        OnTmcReadRequest(value);

    } else if (address == (uint16_t)ModbusRegisters::SystemHoldingRegister::tmc_write_request_value_2) {
        // this is triggered upon writing tmc_write_request_value_2, but its assumed that value1 & value2 are always written together
        OnTmcWriteRequest();

    } else {
        ModbusMessage *msg = (ModbusMessage *)osMailAlloc(m_ModbusQueueHandle, osWaitForever);
        msg->m_Address = address;
        msg->m_Value = value;
        osMailPut(m_ModbusQueueHandle, msg);
    }
}

bool OnReadFIFO(uint16_t address, uint32_t *pValueCount, std::array<uint16_t, MODBUS_FIFO_MAX_COUNT> &fifo) {
    if (address == static_cast<uint16_t>(dwarf::ModbusRegisters::SystemFIFO::encoded_stream)) {
        *pValueCount = handle_encoded_fifo(fifo);
        return true;
    } else {
        return false;
    }
}

void OnTmcReadRequest(uint16_t value) {
    // WARNING: this method is called from different thread

    uint32_t res = stepperE0.read(value);

    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::tmc_read_response_1, (uint16_t)res);
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::tmc_read_response_2, (uint16_t)(res >> 16));

    log_info(ModbusControl, "Read TMC reg=%i, val=%i", value, res);
}

void OnTmcWriteRequest() {
    // WARNING: this method is called from different thread

    uint32_t value = ModbusRegisters::GetRegValue(ModbusRegisters::SystemHoldingRegister::tmc_write_request_value_1) | (ModbusRegisters::GetRegValue(ModbusRegisters::SystemHoldingRegister::tmc_write_request_value_2) << 16);
    uint8_t address = ModbusRegisters::GetRegValue(ModbusRegisters::SystemHoldingRegister::tmc_write_request_address);

    stepperE0.write(address, value);

    log_info(ModbusControl, "Write TMC reg=%i, val=%i", address, value);
}

void ProcessModbusMessages() {
    osEvent event;
    while (true) {
        event = osMailGet(m_ModbusQueueHandle, 0);
        ModbusMessage *msg = (ModbusMessage *)event.value.p;
        if (event.status != osEventMail) {
            break;
        }

        switch (msg->m_Address) {
        case ((uint16_t)ModbusRegisters::SystemCoil::tmc_enable): {
            log_info(ModbusControl, "E stepper enable: %d", msg->m_Value);
            if (msg->m_Value) {
                enable_e_steppers();
            } else {
                disable_e_steppers();
            }
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemCoil::is_selected): {
            SetDwarfSelected(msg->m_Value);
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemCoil::loadcell_enable): {
            loadcell::loadcell_set_enable(msg->m_Value);
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemCoil::accelerometer_enable): {
            dwarf::accelerometer::set_enable(msg->m_Value);
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemCoil::accelerometer_high): {
            dwarf::accelerometer::set_high_sample_rate(msg->m_Value);
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemHoldingRegister::nozzle_target_temperature): {
            log_info(ModbusControl, "Set hotend temperature: %i", msg->m_Value);
            thermalManager.setTargetHotend(msg->m_Value, 0);
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemHoldingRegister::heatbreak_requested_temperature): {
            log_info(ModbusControl, "Set Heatbreak requested temperature: %i", msg->m_Value);
            thermalManager.setTargetHeatbreak(msg->m_Value, 0);
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemHoldingRegister::fan0_pwm): {
            log_info(ModbusControl, "Set print fan PWM:: %i", msg->m_Value);
            thermalManager.set_fan_speed(0, msg->m_Value);
            break;
        }
        case ((uint16_t)ModbusRegisters::SystemHoldingRegister::fan1_pwm): {
            if (msg->m_Value == std::numeric_limits<uint16_t>::max()) {
                // switch back to auto control
                if (Fans::heat_break(0).isSelftest()) {
                    log_info(ModbusControl, "Heatbreak fan: AUTO");
                    Fans::heat_break(0).ExitSelftestMode();
                }
            } else {
                // direct PWM control mode (for selftest)
                if (!Fans::heat_break(0).isSelftest()) {
                    log_info(ModbusControl, "Heatbreak fan: SELFTEST");
                    Fans::heat_break(0).EnterSelftestMode();
                }

                log_info(ModbusControl, "Set heatbreak fan PWM:: %i", msg->m_Value);
                Fans::heat_break(0).SelftestSetPWM(msg->m_Value);
            }

            break;
        }
        case ((uint16_t)ModbusRegisters::SystemHoldingRegister::led_pwm): {
            LedPwm led_pwm = ModbusRegisters::GetRegValue(ModbusRegisters::SystemHoldingRegister::led_pwm);
            if (isDwarfSelected()) {
                Cheese::set_led(led_pwm.selected);
            } else {
                Cheese::set_led(led_pwm.not_selected);
            }
            break;
        }
        }

        osMailFree(m_ModbusQueueHandle, msg);
    }
}

void SetDwarfSelected(bool selected) {
    s_isDwarfSelected = selected;
    LedPwm led_pwm = ModbusRegisters::GetRegValue(ModbusRegisters::SystemHoldingRegister::led_pwm);
    if (selected) {
        Cheese::set_led(led_pwm.selected);
        buddy::hw::localRemote.reset();
    } else {
        buddy::hw::localRemote.set();
        Cheese::set_led(led_pwm.not_selected);
    }
    log_info(ModbusControl, "Dwarf select state: %s", selected ? "YES" : "NO");
}

/**
 * @brief Clamp float temeperature to int16_t, which can be transported in uint16_t modbus register.
 * @param temperature temperature to clamp [deg C]
 * @return clamped temperature [deg C]
 */
static inline int16_t clamp_to_int16(float temperature) {
    return std::clamp(temperature, float(INT16_MIN), float(INT16_MAX));
}

void UpdateRegisters() {
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::hotend_measured_temperature, clamp_to_int16(Temperature::degHotend(0)));
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::hotend_pwm_state, Temperature::getHeaterPower(H_E0));
    ModbusRegisters::SetBitValue(ModbusRegisters::SystemDiscreteInput::is_picked, Cheese::is_picked());
    ModbusRegisters::SetBitValue(ModbusRegisters::SystemDiscreteInput::is_parked, Cheese::is_parked());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::tool_filament_sensor, dwarf::tool_filament_sensor::tool_filament_sensor_get_raw_data());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::mcu_temperature, clamp_to_int16(Temperature::degBoard()));
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::heatbreak_temp, clamp_to_int16(Temperature::degHeatbreak(0)));

    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan0_rpm, Fans::print(0).getActualRPM());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan0_pwm, Fans::print(0).getPWM());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan0_state, Fans::print(0).getState());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan0_is_rpm_ok, Fans::print(0).getRPMIsOk());

    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan1_rpm, Fans::heat_break(0).getActualRPM());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan1_pwm, Fans::heat_break(0).getPWM());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan1_state, Fans::heat_break(0).getState());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fan1_is_rpm_ok, Fans::heat_break(0).getRPMIsOk());

    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::is_picked_raw, Cheese::get_raw_picked());
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::is_parked_raw, Cheese::get_raw_parked());
}

void TriggerMarlinKillFault(dwarf_shared::errors::FaultStatusMask fault, const char *component, const char *message) {
    ModbusRegisters::PutStringIntoInputRegisters(
        ftrstd::to_underlying(ModbusRegisters::SystemInputRegister::marlin_error_component_start),
        ftrstd::to_underlying(ModbusRegisters::SystemInputRegister::marlin_error_component_end),
        component);

    ModbusRegisters::PutStringIntoInputRegisters(
        ftrstd::to_underlying(ModbusRegisters::SystemInputRegister::marlin_error_message_start),
        ftrstd::to_underlying(ModbusRegisters::SystemInputRegister::marlin_error_message_end),
        message);

    // this erases existing erros, but since this is fatal error, it doesn't matter
    ModbusRegisters::SetRegValue(ModbusRegisters::SystemInputRegister::fault_status, static_cast<uint16_t>(fault));
}

} // namespace dwarf::ModbusControl
