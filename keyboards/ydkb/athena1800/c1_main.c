#include "ch.h"
#include "hal.h"
#include "config.h"

// Main process for core1
static THD_WORKING_AREA(wa_c1_main_task_wrapper, 2048);
static THD_FUNCTION(c1_main_task_wrapper, arg)
{
    while (1) {
        c1_main_task();
        chThdSleepMicroseconds(500);
    }
}

void c1_main_task(void)
{
#ifdef QUANTUM_PAINTER_ENABLE
    // Run Quantum Painter task
    void qp_internal_task(void);
    qp_internal_task();
#endif
    display_task_user();
}

// Entry point of core1
void c1_main(void)
{
    chSysWaitSystemState(ch_sys_running);
    chInstanceObjectInit(&ch1, &ch_core1_cfg);
    chSysUnlock();

    display_init();
    // Start main task
    chThdCreateStatic(wa_c1_main_task_wrapper, sizeof(wa_c1_main_task_wrapper), NORMALPRIO + 1, c1_main_task_wrapper, NULL);
}
