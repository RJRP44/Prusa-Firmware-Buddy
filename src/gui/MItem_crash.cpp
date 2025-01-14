#include "config_features.h"
// TODO do it in cmake
#if ENABLED(CRASH_RECOVERY)

    #include "MItem_crash.hpp"
    #include "menu_spin_config.hpp"
    #include "eeprom.h"
    #include "../lib/Marlin/Marlin/src/module/stepper/trinamic.h"
    #include "../Marlin/src/feature/prusa/crash_recovery.h"

MI_CRASH_DETECTION::MI_CRASH_DETECTION()
    : WI_ICON_SWITCH_OFF_ON_t(0, _(label), nullptr, is_enabled_t::yes,
    #if ((PRINTER_TYPE == PRINTER_PRUSA_MK4))
        is_hidden_t::dev
    #else
        is_hidden_t::no
    #endif // ((PRINTER_TYPE == PRINTER_PRUSA_MK4))
    ) {
    index = crash_s.is_enabled();
}

void MI_CRASH_DETECTION::OnChange([[maybe_unused]] size_t old_index) {
    crash_s.enable(index);
}

MI_CRASH_SENSITIVITY_X::MI_CRASH_SENSITIVITY_X()
    : WiSpinInt(crash_s.get_sensitivity().x, SpinCnf::crash_sensitivity,
        _(label), nullptr, is_enabled_t::yes,
    #if PRINTER_TYPE == PRINTER_PRUSA_XL
        is_hidden_t::no
    #else
        is_hidden_t::dev
    #endif
    ) {
}
void MI_CRASH_SENSITIVITY_X::OnClick() {

    xy_long_t se = crash_s.get_sensitivity();
    se.x = GetVal();
    crash_s.set_sensitivity(se);
}

MI_CRASH_SENSITIVITY_Y::MI_CRASH_SENSITIVITY_Y()
    : WiSpinInt(crash_s.get_sensitivity().y, SpinCnf::crash_sensitivity,
        _(label), nullptr, is_enabled_t::yes,
    #if PRINTER_TYPE == PRINTER_PRUSA_XL
        is_hidden_t::no
    #else
        is_hidden_t::dev
    #endif
    ) {
}
void MI_CRASH_SENSITIVITY_Y::OnClick() {

    xy_long_t se = crash_s.get_sensitivity();
    se.y = GetVal();
    crash_s.set_sensitivity(se);
}

    #if (PRINTER_TYPE == PRINTER_PRUSA_XL)
MI_CRASH_SENSITIVITY_XY::MI_CRASH_SENSITIVITY_XY()
    : WI_SWITCH_t<3>(get_item_id_from_sensitivity(crash_s.get_sensitivity().x), _(label), nullptr, is_enabled_t::yes, is_hidden_t::no,
        _(ITEMS[0].name), _(ITEMS[1].name), _(ITEMS[2].name)) {}

constexpr size_t MI_CRASH_SENSITIVITY_XY::get_item_id_from_sensitivity(int32_t sensitivity) {
    for (size_t i = 0; i <= std::size(ITEMS); i++) {
        if (ITEMS[i].value == sensitivity)
            return i;
    }
    return 0;
}

void MI_CRASH_SENSITIVITY_XY::OnChange([[maybe_unused]] size_t old_index) {
    int32_t sensitivity = ITEMS[index].value;
    xy_long_t se = crash_s.get_sensitivity();
    se.x = sensitivity;
    se.y = sensitivity;
    crash_s.set_sensitivity(se);
}
    #else
MI_CRASH_SENSITIVITY_XY::MI_CRASH_SENSITIVITY_XY()
    : WiSpinInt(crash_s.get_sensitivity().x, SpinCnf::crash_sensitivity,
        _(label), nullptr, is_enabled_t::yes,
        is_hidden_t::dev) {
}
void MI_CRASH_SENSITIVITY_XY::OnClick() {

    xy_long_t se = crash_s.get_sensitivity();
    se.x = GetVal();
    se.y = GetVal();
    crash_s.set_sensitivity(se);
}
    #endif

MI_CRASH_MAX_PERIOD_X::MI_CRASH_MAX_PERIOD_X()
    : WI_SPIN_CRASH_PERIOD_t(crash_s.get_max_period().x, SpinCnf::crash_max_period, _(label), nullptr, is_enabled_t::yes, is_hidden_t::dev) {
}
void MI_CRASH_MAX_PERIOD_X::OnClick() {
    xy_long_t mp = crash_s.get_max_period();
    mp.x = GetVal();
    crash_s.set_max_period(mp);
}

MI_CRASH_MAX_PERIOD_Y::MI_CRASH_MAX_PERIOD_Y()
    : WI_SPIN_CRASH_PERIOD_t(crash_s.get_max_period().y, SpinCnf::crash_max_period, _(label), nullptr, is_enabled_t::yes, is_hidden_t::dev) {
}
void MI_CRASH_MAX_PERIOD_Y::OnClick() {
    xy_long_t mp = crash_s.get_max_period();
    mp.y = GetVal();
    crash_s.set_max_period(mp);
}

#endif // ENABLED(CRASH_RECOVERY)

#if ANY(CRASH_RECOVERY, POWER_PANIC)

MI_POWER_PANICS::MI_POWER_PANICS()
    : WI_INFO_t(variant8_get_ui16(eeprom_get_var(EEVAR_POWER_COUNT_TOT)), _(label)) {}

MI_CRASHES_X_LAST::MI_CRASHES_X_LAST()
    : WI_INFO_t(crash_s.counter_crash.x, _(label)) {}

MI_CRASHES_Y_LAST::MI_CRASHES_Y_LAST()
    : WI_INFO_t(crash_s.counter_crash.y, _(label)) {}

MI_CRASHES_X::MI_CRASHES_X()
    : WI_INFO_t(variant8_get_ui16(eeprom_get_var(EEVAR_CRASH_COUNT_X_TOT)), _(label)) {}

MI_CRASHES_Y::MI_CRASHES_Y()
    : WI_INFO_t(variant8_get_ui16(eeprom_get_var(EEVAR_CRASH_COUNT_Y_TOT)), _(label)) {}

    #if HAS_DRIVER(TMC2130)
MI_CRASH_FILTERING::MI_CRASH_FILTERING()
    : WI_ICON_SWITCH_OFF_ON_t(0, _(label), nullptr, is_enabled_t::yes, is_hidden_t::dev) {
    index = crash_s.get_filter();
}

void MI_CRASH_FILTERING::OnChange([[maybe_unused]] size_t old_index) {
    crash_s.set_filter(index);
}
    #endif
#endif // ANY(CRASH_RECOVERY, POWER_PANIC)
