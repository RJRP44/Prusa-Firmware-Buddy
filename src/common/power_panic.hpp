#pragma once
#include <stdint.h>
#include "cmsis_os.h"

#include "Marlin/src/feature/prusa/crash_recovery.h"

namespace power_panic {

enum class PPState : uint8_t {
    Inactive,
    Prepared,
    Triggered,
    Retracting,
    SaveState,
    WaitingToDie,
};

// A power panic is triggered only in the event of an AC power failure in the print state
// ac_power_fault_is_checked is set in all cases of AC power failure (it is used to disable EEPROM writing)
extern std::atomic_bool ac_power_fault_is_checked;

// TODO: internal state can be hidden by improving the interface
extern PPState power_panic_state;

// Return true if print state has been stored
bool state_stored();

/// Load the powerpanic state and setup print progress
/// @returns True if print can auto-recover
bool setup_auto_recover_check();

// Return the SFN media path of the print to be resumed
const char *stored_media_path();

// Clear existing stored state, reset internal state
void reset();

// Prepare the fixed data section in the flash, including MBL and filename data.
// Call once after MBL during the printing process (usually after a successful G29)
void prepare();

// Start resuming a stored print
void resume_print(bool start_paused);

// Actually resume the print after starting paused
void resume_continue();

// Main resume loop handler
void resume_loop();

// Main fault loop handler
void ac_fault_loop();

// AC fault ISR handler
void ac_fault_isr();

// AC fault Task handler
extern osThreadId ac_fault_task;

void ac_fault_main(void const *argument);

// Current acFault pin status
bool is_ac_fault_signal();

};
