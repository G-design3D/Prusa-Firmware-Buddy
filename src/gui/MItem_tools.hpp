/*****************************************************************************/
// menu items running tools
#pragma once
#include "WindowMenuItems.hpp"
#include "i18n.h"
#include "filament.hpp"
#include "WindowItemFormatableLabel.hpp"
#include "WindowItemFanLabel.hpp"
#include "WindowItemTempLabel.hpp"
#include "config.h"
#include <utility_extensions.hpp>
#include <option/has_dwarf.h>

class MI_FILAMENT_SENSOR : public WI_ICON_SWITCH_OFF_ON_t {
    constexpr static const char *const label = N_("Filament Sensor");

    bool init_index() const;

public:
    MI_FILAMENT_SENSOR()
        : WI_ICON_SWITCH_OFF_ON_t(init_index(), _(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {}

protected:
    virtual void OnChange(size_t old_index) override;
};

class MI_LIVE_ADJUST_Z : public WI_LABEL_t {
    static constexpr const char *const label = N_("Live Adjust Z");

public:
    MI_LIVE_ADJUST_Z();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_AUTO_HOME : public WI_LABEL_t {
    static constexpr const char *const label = N_("Auto Home");

public:
    MI_AUTO_HOME();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_MESH_BED : public WI_LABEL_t {
    static constexpr const char *const label = N_("Mesh Bed Leveling");

public:
    MI_MESH_BED();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_CALIB_Z : public WI_LABEL_t {
    static constexpr const char *const label = N_("Calibrate Z");

public:
    MI_CALIB_Z();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_DISABLE_STEP : public WI_LABEL_t {
    static constexpr const char *const label = N_("Disable Motors");

public:
    MI_DISABLE_STEP();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_FACTORY_SOFT_RESET : public WI_LABEL_t {
    static constexpr const char *const label = N_("Reset Settings & Calibrations");

public:
    MI_FACTORY_SOFT_RESET();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_FACTORY_HARD_RESET : public WI_LABEL_t {
    static constexpr const char *const label = N_("Hard Reset (USB with FW needed)");

public:
    MI_FACTORY_HARD_RESET();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

#ifdef BUDDY_ENABLE_DFU_ENTRY
class MI_ENTER_DFU : public WI_LABEL_t {
    static constexpr const char *const label = "Enter DFU";

public:
    MI_ENTER_DFU();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};
#endif

class MI_SAVE_DUMP : public WI_LABEL_t {
    static constexpr const char *const label = N_("Save Crash Dump");

public:
    MI_SAVE_DUMP();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_XFLASH_RESET : public WI_LABEL_t {
    static constexpr const char *const label = "Delete Crash Dump"; // intentionally not translated, only for debugging

public:
    MI_XFLASH_RESET();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_EE_SAVEXML : public WI_LABEL_t {
    static constexpr const char *const label = "TODO EE Save XML"; // intentionally not translated, only for debugging

public:
    MI_EE_SAVEXML();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_EE_CLEAR : public WI_LABEL_t {
    static constexpr const char *const label = "EE Clear"; // intentionally not translated, only for debugging

public:
    MI_EE_CLEAR();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_M600 : public WI_LABEL_t {
    static constexpr const char *const label = N_("Change Filament");

public:
    MI_M600();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_TIMEOUT : public WI_ICON_SWITCH_OFF_ON_t {
    constexpr static const char *const label = N_("Menu Timeout");

public:
    MI_TIMEOUT();
    virtual void OnChange(size_t old_index) override;
};

#ifdef _DEBUG
inline constexpr size_t MI_SOUND_MODE_COUNT = 5;
#else
inline constexpr size_t MI_SOUND_MODE_COUNT = 4;
#endif
class MI_SOUND_MODE : public WI_SWITCH_t<MI_SOUND_MODE_COUNT> {
    constexpr static const char *const label = N_("Sound Mode");

    constexpr static const char *str_Once = N_("Once");
    constexpr static const char *str_Loud = N_("Loud");
    constexpr static const char *str_Silent = N_("Silent");
    constexpr static const char *str_Assist = N_("Assist");
    constexpr static const char *str_Debug = "Debug";

    size_t init_index() const;

public:
    MI_SOUND_MODE();
    virtual void OnChange(size_t old_index) override;
};

class MI_SOUND_TYPE : public WI_SWITCH_t<8> {
    constexpr static const char *const label = "Sound Type";

    constexpr static const char *str_ButtonEcho = "ButtonEcho";
    constexpr static const char *str_StandardPrompt = "StandardPrompt";
    constexpr static const char *str_StandardAlert = "StandardAlert";
    constexpr static const char *str_CriticalAlert = "CriticalAlert";
    constexpr static const char *str_EncoderMove = "EncoderMove";
    constexpr static const char *str_BlindAlert = "BlindAlert";
    constexpr static const char *str_Start = "Start";
    constexpr static const char *str_SingleBeep = "SingleBeep";

public:
    MI_SOUND_TYPE();
    virtual void OnChange(size_t old_index) override;
};

class MI_SORT_FILES : public WI_SWITCH_t<2> {
    constexpr static const char *const label = N_("Sort Files");

    constexpr static const char *str_name = N_("Name");
    constexpr static const char *str_time = N_("Time");

public:
    MI_SORT_FILES();
    virtual void OnChange(size_t old_index) override;
};

class MI_SOUND_VOLUME : public WiSpinInt {
    constexpr static const char *const label = N_("Sound Volume");

public:
    MI_SOUND_VOLUME();
    virtual void OnClick() override;
    /* virtual void Change() override; */
};

class MI_TIMEZONE : public WiSpinInt {
    constexpr static const char *const label = N_("Time Zone Offset"); // intentionally not translated

public:
    MI_TIMEZONE();
    virtual void OnClick() override;
};

class MI_TIME_FORMAT : public WI_SWITCH_t<2> {
    constexpr static const char *const label = N_("Time Format"); // intentionally not translated

    constexpr static const char *str_24h = N_("24h");
    constexpr static const char *str_12h = N_("12h");

public:
    MI_TIME_FORMAT();
    virtual void OnChange(size_t old_index) override;
};

// TODO move to different files (filament sensor adc related ones ...)
class IMI_FS_SPAN : public WiSpinInt {
    bool is_side;
    size_t index;

public:
    IMI_FS_SPAN(bool is_side_, size_t index, const char *label);
    virtual void OnClick() override;
};

template <size_t Index, bool IsSide>
class MI_FS_SPAN : public IMI_FS_SPAN {
    static_assert(Index < 6, "Index out of range");
    struct index_data {
        const char *label;
        size_t extruder_index;
    };

    static consteval const char *get_label() {
        // gui counts sensors from 1, but internally they are counted from 0
        if (IsSide) {
            switch (Index) {
            case 0:
                return N_("Side FS span 1");
            case 1:
                return N_("Side FS span 2");
            case 2:
                return N_("Side FS span 3");
            case 3:
                return N_("Side FS span 4");
            case 4:
                return N_("Side FS span 5");
            case 5:
                return N_("Side FS span 6");
            default:
                consteval_assert_false();
                return ""; // cannot happen
            }
        } else {
            switch (Index) {
            case 0:
                return N_("FS span 1");
            case 1:
                return N_("FS span 2");
            case 2:
                return N_("FS span 3");
            case 3:
                return N_("FS span 4");
            case 4:
                return N_("FS span 5");
            case 5:
                return N_("FS span 6");
            default:
                consteval_assert_false();
                return ""; // cannot happen
            }
        }
    }

public:
    MI_FS_SPAN()
        : IMI_FS_SPAN(IsSide, Index, get_label()) {}
};

class IMI_FS_REF : public WiSpinInt {
    bool is_side;
    size_t index;

public:
    IMI_FS_REF(bool is_side_, size_t index, const char *label);
    virtual void OnClick() override;
};

template <size_t Index, bool IsSide>
class MI_FS_REF : public IMI_FS_REF {
    static_assert(Index < 6, "Index out of range");
    struct index_data {
        const char *label;
        size_t extruder_index;
    };

    static consteval const char *get_label() {
        // gui counts sensors from 1, but internally they are counted from 0
        if (IsSide) {
            switch (Index) {
            case 0:
                return N_("Side FS ref 1");
            case 1:
                return N_("Side FS ref 2");
            case 2:
                return N_("Side FS ref 3");
            case 3:
                return N_("Side FS ref 4");
            case 4:
                return N_("Side FS ref 5");
            case 5:
                return N_("Side FS ref 6");
            default:
                consteval_assert_false();
                return ""; // cannot happen
            }
        } else {
            switch (Index) {
            case 0:
                return N_("FS ref 1");
            case 1:
                return N_("FS ref 2");
            case 2:
                return N_("FS ref 3");
            case 3:
                return N_("FS ref 4");
            case 4:
                return N_("FS ref 5");
            case 5:
                return N_("FS ref 6");
            default:
                consteval_assert_false();
                return ""; // cannot happen
            }
        }
    }

public:
    MI_FS_REF()
        : IMI_FS_REF(IsSide, Index, get_label()) {}
};

class MI_FAN_CHECK : public WI_ICON_SWITCH_OFF_ON_t {
    constexpr static const char *const label = N_("Fan Check");

public:
    MI_FAN_CHECK();
    virtual void OnChange(size_t old_index) override;
};

/******************************************************************/
// WI_INFO_t
class MI_INFO_FW : public WI_INFO_t {
    static constexpr const char *const label = N_("Firmware Version");

public:
    MI_INFO_FW();
    virtual void click(IWindowMenu &window_menu) override; // FW version is clickable when development tools are shown
};

class MI_INFO_BOOTLOADER : public WI_INFO_t {
    static constexpr const char *const label = N_("Bootloader Version");

public:
    MI_INFO_BOOTLOADER();
};

class MI_INFO_BOARD : public WI_INFO_t {
    static constexpr const char *const label = N_("Buddy Board");

public:
    MI_INFO_BOARD();
};

class MI_INFO_SERIAL_NUM : public WiInfo<28> {
    static constexpr const char *const label = N_("Serial Number");

public:
    MI_INFO_SERIAL_NUM();
};

class MI_METRICS_ENABLE : public WI_ICON_SWITCH_OFF_ON_t {
    static constexpr const char *const label = N_("Allow Metrics");
    static constexpr const char *const txt_confirm = N_("This will allow metrics to be enabled by G-code. It can send unencrypted diagnostics data to the internet. Do you really want to allow metrics?");

public:
    MI_METRICS_ENABLE();
    virtual void OnChange(size_t old_index) override;
};

class MI_FS_AUTOLOAD : public WI_ICON_SWITCH_OFF_ON_t {
    constexpr static const char *const label = N_("Filament Autoloading");

public:
    MI_FS_AUTOLOAD();
    virtual void OnChange(size_t old_index) override;
};

class MI_INFO_BED_TEMP : public WI_TEMP_LABEL_t {
    static constexpr const char *const label = N_("Bed Temperature");

public:
    MI_INFO_BED_TEMP();
};

class MI_INFO_FILL_SENSOR : public WI_FORMATABLE_LABEL_t<std::pair<SensorData::Value, SensorData::Value>> {
public:
    MI_INFO_FILL_SENSOR(string_view_utf8 label);
};

class MI_INFO_PRINTER_FILL_SENSOR : public MI_INFO_FILL_SENSOR {
    static constexpr const char *label =
#if PRINTER_IS_PRUSA_XL
        N_("Tool Filament sensor");
#else
        N_("Filament Sensor");
#endif

public:
    MI_INFO_PRINTER_FILL_SENSOR();
};

class MI_INFO_SIDE_FILL_SENSOR : public MI_INFO_FILL_SENSOR {
    static constexpr const char *label = N_("Side Filament Sensor");

public:
    MI_INFO_SIDE_FILL_SENSOR();
};

class MI_INFO_PRINT_FAN : public WI_FAN_LABEL_t {
    static constexpr const char *const label = N_("Print Fan");

public:
    MI_INFO_PRINT_FAN();
};

class MI_INFO_HBR_FAN : public WI_FAN_LABEL_t {
    static constexpr const char *const label = N_("Heatbreak Fan");

public:
    MI_INFO_HBR_FAN();
};

class MI_PRINT_PROGRESS_TIME : public WiSpinInt {
    constexpr static const char *label = N_("Print Progress Screen");

public:
    MI_PRINT_PROGRESS_TIME();
    virtual void OnClick() override;
};
class MI_ODOMETER_DIST : public WI_FORMATABLE_LABEL_t<float> {
public:
    MI_ODOMETER_DIST(string_view_utf8 label, const img::Resource *icon, is_enabled_t enabled, is_hidden_t hidden, float initVal);
};

class MI_ODOMETER_DIST_X : public MI_ODOMETER_DIST {
    constexpr static const char *const label = N_("X Axis");

public:
    MI_ODOMETER_DIST_X();
};
class MI_ODOMETER_DIST_Y : public MI_ODOMETER_DIST {
    constexpr static const char *const label = N_("Y Axis");

public:
    MI_ODOMETER_DIST_Y();
};
class MI_ODOMETER_DIST_Z : public MI_ODOMETER_DIST {
    constexpr static const char *const label = N_("Z Axis");

public:
    MI_ODOMETER_DIST_Z();
};

/// Extruded filament
class MI_ODOMETER_DIST_E : public MI_ODOMETER_DIST {
    constexpr static const char *const generic_label = N_("Filament");

public:
    MI_ODOMETER_DIST_E(const char *const label, int index);
    MI_ODOMETER_DIST_E();
};

/// Tool picked
class MI_ODOMETER_TOOL : public WI_FORMATABLE_LABEL_t<uint32_t> {
    constexpr static const char *const generic_label = N_("Tools Changed");
    constexpr static const char *const times_label = N_("times"); // Tools Changed      123 times

public:
    MI_ODOMETER_TOOL(const char *const label, int index);
    MI_ODOMETER_TOOL();
};

class MI_ODOMETER_TIME : public WI_FORMATABLE_LABEL_t<uint32_t> {
    constexpr static const char *const label = N_("Print Time");

public:
    MI_ODOMETER_TIME();
};

class MI_INFO_HEATER_VOLTAGE : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("Heater Voltage");

public:
    MI_INFO_HEATER_VOLTAGE();
};

class MI_INFO_INPUT_VOLTAGE : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("Input Voltage");

public:
    MI_INFO_INPUT_VOLTAGE();
};

class MI_INFO_5V_VOLTAGE : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("5V Voltage");

public:
    MI_INFO_5V_VOLTAGE();
};

class MI_INFO_HEATER_CURRENT : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("Heater Current");

public:
    MI_INFO_HEATER_CURRENT();
};

class MI_INFO_INPUT_CURRENT : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("Input Current");

public:
    MI_INFO_INPUT_CURRENT();
};

class MI_INFO_MMU_CURRENT : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("MMU Current");

public:
    MI_INFO_MMU_CURRENT();
};

class MI_INFO_SPLITTER_5V_CURRENT : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("Splitter 5V Current");

public:
    MI_INFO_SPLITTER_5V_CURRENT();
};

class MI_INFO_SANDWICH_5V_CURRENT : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("Sandwich 5V Current");

public:
    MI_INFO_SANDWICH_5V_CURRENT();
};

class MI_INFO_BUDDY_5V_CURRENT : public WI_FORMATABLE_LABEL_t<SensorData::Value> {
    static constexpr const char *const label = N_("XL Buddy 5V Current");

public:
    MI_INFO_BUDDY_5V_CURRENT();
};

class MI_INFO_BOARD_TEMP : public WI_TEMP_LABEL_t {
    static constexpr const char *const label = N_("Board Temperature");

public:
    MI_INFO_BOARD_TEMP();
};

class MI_FOOTER_RESET : public WI_LABEL_t {
    static constexpr const char *const label = N_("Reset");

public:
    MI_FOOTER_RESET();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_CO_CANCEL_OBJECT : public WI_LABEL_t {
    static constexpr const char *const label = N_("Cancel Object (Experimental)");

public:
    MI_CO_CANCEL_OBJECT();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_HEATUP_BED : public WI_SWITCH_t<2> {
    static constexpr const char *const label = N_("For filament change, preheat");
    static constexpr const char *const nozzle = N_("Nozzle");
    static constexpr const char *const nozzle_bed = N_("Noz&Bed");

public:
    MI_HEATUP_BED();

protected:
    void OnChange(size_t old_index) override;
};

/******************************************************************/

enum class input_shaper_param {
    set_values,
    change_x,
    change_y
};

class MI_IS_X_ONOFF : public WI_ICON_SWITCH_OFF_ON_t {
    static constexpr const char *const label = N_("X-axis");
    input_shaper_param param = input_shaper_param::change_x;

public:
    MI_IS_X_ONOFF();

protected:
    void OnChange(size_t old_index) override;
};

class MI_IS_Y_ONOFF : public WI_ICON_SWITCH_OFF_ON_t {
    static constexpr const char *const label = N_("Y-axis");
    input_shaper_param param = input_shaper_param::change_y;

public:
    MI_IS_Y_ONOFF();

protected:
    void OnChange(size_t old_index) override;
};

class MI_IS_X_TYPE : public WI_SWITCH_t<6> {
    static constexpr const char *const label = N_("X-axis filter");

public:
    MI_IS_X_TYPE();

protected:
    void OnChange(size_t old_index) override;
};

class MI_IS_Y_TYPE : public WI_SWITCH_t<6> {
    static constexpr const char *const label = N_("Y-axis filter");

public:
    MI_IS_Y_TYPE();

protected:
    void OnChange(size_t old_index) override;
};

class MI_IS_X_FREQUENCY : public WiSpinInt {
    static constexpr const char *const label = N_("X-axis freq.");

public:
    MI_IS_X_FREQUENCY();

    virtual void OnClick() override;
};

class MI_IS_Y_FREQUENCY : public WiSpinInt {
    static constexpr const char *const label = N_("Y-axis freq.");

public:
    MI_IS_Y_FREQUENCY();
    virtual void OnClick() override;
};

class MI_IS_Y_COMPENSATION : public WI_ICON_SWITCH_OFF_ON_t {
    static constexpr const char *const label = N_("Y weight compensation");

public:
    MI_IS_Y_COMPENSATION();

protected:
    void OnChange(size_t old_index) override;
};

class MI_IS_SET : public WI_LABEL_t {
    static constexpr const char *const label = N_("Set up values");
    input_shaper_param param = input_shaper_param::set_values;

public:
    MI_IS_SET();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_IS_CALIB : public WI_LABEL_t {
    static constexpr const char *const label = N_("Calibration");

public:
    MI_IS_CALIB();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};
