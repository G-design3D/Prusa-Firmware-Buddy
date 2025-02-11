/**
 * @file
 */
#include <cstring> // memset
#include <stdint.h>
#include <assert.h>
#include <climits>
#include <atomic>

#include "filament_sensor_adc.hpp"
#include "filament_sensors_handler.hpp"
#include "log.h"
#include "metric.h"
#include "config_buddy_2209_02.h"
#include "algorithm_range.hpp"
#include "rtos_api.hpp"
#include <config_store/store_instance.hpp>

LOG_COMPONENT_REF(FSensor);

namespace {

constexpr int32_t fs_disconnect_threshold = // value for detecting disconnected sensor
#if (BOARD_IS_XLBUDDY)
    20;
#else
    2000;
#endif

} // unnamed namespace

void FSensorADC::enable() {
    state = fsensor_t::NotInitialized;
}

void FSensorADC::disable() {
    state = fsensor_t::Disabled;
}

void FSensorADC::cycle() {
    const auto filtered_value { fs_filtered_value.load() }; // store value - so interrupt cannot change it during evaluation

    if (flg_load_settings) {
        load_settings();
        init(); // will enable or disable, depends on eeprom disable flag
    }
    if (flg_invalid_calib) {
        invalidate_calibration();
        Disable();
    }
    if (req_calibrate == CalibrateRequest::CalibrateNoFilament) {
        save_calibration(filtered_value);
        Enable();
    }

    if (req_calibrate == CalibrateRequest::CalibrateHasFilament) {
        EnsureHasFilamentValue(filtered_value);
    }

    // disabled FS will not enter cycle, but load_settings can disable it too
    // so better not try to change state when sensor is disabled
    if (state != fsensor_t::Disabled) {
        state = evaluate_state(filtered_value);
    }
}

void FSensorADC::set_filtered_value_from_IRQ(int32_t filtered_value) {
    fs_filtered_value.store(filtered_value);
}

void FSensorADC::set_state(fsensor_t st) {
    CriticalSection C;
    state = st;
}

FSensorADC::FSensorADC(uint8_t tool_index, bool is_side_sensor)
    : tool_index(tool_index)
    , is_side(is_side_sensor) {
    load_settings();
    init();
}

void FSensorADC::SetCalibrateRequest(CalibrateRequest req) {
    req_calibrate = req;
}

bool FSensorADC::IsCalibrationFinished() const {
    return req_calibrate == CalibrateRequest::NoCalibration;
}

void FSensorADC::SetLoadSettingsFlag() {
    flg_load_settings = true;
}

void FSensorADC::SetInvalidateCalibrationFlag() {
    flg_invalid_calib = true;
}

void FSensorADC::EnsureHasFilamentValue(int32_t filtered_value) {
    if (filtered_value == fs_filtered_value_not_ready) {
        return;
    }

    // value should be outside of extended span, because if its close to span that is used to evaluate filament sensor, it will not be reliable and trigger randomly
    int32_t extended_span = fs_value_span * fs_selftest_span_multipler;
    if (IsInClosedRange(filtered_value, fs_ref_value - extended_span, fs_ref_value + extended_span)) {
        log_info(FSensor, "Calibrating HasFilament: FAIL value: %d", filtered_value);
        invalidate_calibration();
    } else {
        log_info(FSensor, "Calibrating HasFilament: PASS value: %d", filtered_value);
    }

    // mark calibration as done
    req_calibrate = CalibrateRequest::NoCalibration;
}

int32_t FSensorADC::load_settings() {
    fs_value_span = is_side ? config_store().get_side_fs_value_span(tool_index) : config_store().get_extruder_fs_value_span(tool_index);
    fs_ref_value = is_side ? config_store().get_side_fs_ref_value(tool_index) : config_store().get_extruder_fs_ref_value(tool_index);
    flg_load_settings = false;
    return fs_ref_value;
}

fsensor_t FSensorADC::evaluate_state(int32_t filtered_value) {
    if (filtered_value == fs_filtered_value_not_ready) {
        return fsensor_t::NotInitialized;
    }
    if (fs_ref_value == fs_ref_value_not_calibrated) {
        return fsensor_t::NotCalibrated;
    }
    if (filtered_value < fs_disconnect_threshold) {
        return fsensor_t::NotConnected;
    }
    if (IsInClosedRange(filtered_value, fs_ref_value - fs_value_span, fs_ref_value + fs_value_span)) {
        return fsensor_t::NoFilament;
    }
    return fsensor_t::HasFilament;
}

void FSensorADC::save_calibration(int32_t value) {
    if (value == fs_filtered_value_not_ready) {
        return;
    }

    if (is_side) {
        config_store().set_side_fs_ref_value(tool_index, value);
    } else {
        config_store().set_extruder_fs_ref_value(tool_index, value);
    }
    req_calibrate = CalibrateRequest::NoCalibration;
    load_settings();

    log_info(FSensor, "Calibrating NoFilament value: %d", value);
}

void FSensorADC::invalidate_calibration() {
    if (is_side) {
        config_store().set_side_fs_ref_value(tool_index, fs_ref_value_not_calibrated);
    } else {
        config_store().set_extruder_fs_ref_value(tool_index, fs_ref_value_not_calibrated);
    }
    flg_invalid_calib = false;
    load_settings();
}

void FSensorAdcExtruder::record_state() {
    if (limit_record()) {
        metric_record_custom(&get_metric__static(), ",n=%u st=%di,f=%di,r=%di,sp=%di", tool_index, static_cast<int>(Get()), fs_filtered_value.load(), fs_ref_value, fs_value_span);
    }
}

void FSensorAdcSide::record_state() {
    if (limit_record()) {
        metric_record_custom(&get_metric__static(), ",n=%u st=%di,f=%di,r=%di,sp=%di", tool_index, static_cast<int>(Get()), fs_filtered_value.load(), fs_ref_value, fs_value_span);
    }
}

void FSensorAdcExtruder::MetricsSetEnabled(bool enable) {
    uint32_t new_state = enable ? METRIC_HANDLER_ENABLE_ALL : METRIC_HANDLER_DISABLE_ALL;
    get_metric_raw__static().enabled_handlers = new_state;
    get_metric__static().enabled_handlers = new_state;
}
void FSensorAdcSide::MetricsSetEnabled(bool enable) {
    uint32_t new_state = enable ? METRIC_HANDLER_ENABLE_ALL : METRIC_HANDLER_DISABLE_ALL;
    get_metric_raw__static().enabled_handlers = new_state;
    get_metric__static().enabled_handlers = new_state;
}

void FSensorAdcExtruder::record_raw(int32_t val) {
    if (limit_record_raw()) {
        metric_record_custom(&get_metric_raw__static(), ",n=%u v=%di", tool_index, val);
    }
}

void FSensorAdcSide::record_raw(int32_t val) {
    if (limit_record_raw()) {
        metric_record_custom(&get_metric_raw__static(), ",n=%u v=%di", tool_index, val);
    }
}

#define METRIC_HANDLER METRIC_HANDLER_DISABLE_ALL

metric_s &FSensorAdcExtruder::get_metric_raw__static() {
    static metric_t ret = METRIC("fsensor_raw", METRIC_VALUE_CUSTOM, 0, METRIC_HANDLER); // No min interval, is handled by limit_record_raw
    return ret;
}

metric_s &FSensorAdcExtruder::get_metric__static() {
    static metric_t ret = METRIC("fsensor", METRIC_VALUE_CUSTOM, 0, METRIC_HANDLER); // No min interval, is handled by limit_record_state
    return ret;
}

metric_s &FSensorAdcSide::get_metric_raw__static() {
    static metric_t ret = METRIC("side_fsensor_raw", METRIC_VALUE_CUSTOM, 0, METRIC_HANDLER); // No min interval, is handled by limit_record_raw
    return ret;
}

metric_s &FSensorAdcSide::get_metric__static() {
    static metric_t ret = METRIC("side_fsensor", METRIC_VALUE_CUSTOM, 0, METRIC_HANDLER); // No min interval, is handled by limit_record_state
    return ret;
}

FSensorAdcExtruder::FSensorAdcExtruder(uint8_t tool_index, bool is_side_sensor)
    : FSensorADC(tool_index, is_side_sensor)
    , limit_record(49)
    , limit_record_raw(60) {}

FSensorAdcSide::FSensorAdcSide(uint8_t tool_index, bool is_side_sensor)
    : FSensorADC(tool_index, is_side_sensor)
    , limit_record(49)
    , limit_record_raw(60) {}
