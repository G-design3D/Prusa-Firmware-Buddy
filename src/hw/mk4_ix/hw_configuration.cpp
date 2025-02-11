/**
 * @file hw_configuration.cpp
 */

#include "hw_configuration.hpp"
#include "at21csxx_otp.hpp"
#include "otp.hpp"
#include <device/hal.h>

/**
 * @brief use this  function only once during startup!!!
 * currently LoveBoardEeprom has to be OTP_v2
 *
 * @return LoveBoardEeprom data from loveboards eeprom
 */
static LoveBoardEeprom read_loveboard() {
    __HAL_RCC_GPIOF_CLK_ENABLE(); // enable loveboard eeprom pin port clock
    OtpFromEeprom LoveBoard = OtpFromEeprom(GPIOF, GPIO_PIN_13);
    return LoveBoard.calib_data;
}

namespace buddy::hw {

Configuration &Configuration::Instance() {
    static Configuration ths = Configuration(read_loveboard());
    return ths;
}

Configuration::Configuration(const LoveBoardEeprom &loveboard)
    : loveboard_eeprom(loveboard) {
}

float Configuration::curr_measurement_voltage_to_current(float voltage) const {
    constexpr float allegro_curr_from_voltage = 1 / 0.09F;

    const float allegro_zero_curr_voltage = (get_board_bom_id() == 27) ? 5.F / 2.F : 3.35F / 2.F; // choose half of 3V3 or 5V range

    return (voltage - allegro_zero_curr_voltage) * allegro_curr_from_voltage;
}

} // namespace buddy::hw
