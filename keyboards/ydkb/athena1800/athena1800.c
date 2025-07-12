#include "qp.h"
#include "qp_comms.h"

int main(void) {
    platform_setup();
    protocol_setup();
    keyboard_setup();

    protocol_pre_init();
    keyboard_init();
    protocol_post_init();

#if (RP_CORE1_START != TRUE)
    display_init();
#endif
    /* Main loop */
    while (true) {
        protocol_pre_task();
        protocol_keyboard_task();
        protocol_post_task();

#ifdef RAW_ENABLE
        void raw_hid_task(void);
        raw_hid_task();
#endif

#ifdef CONSOLE_ENABLE
        void console_task(void);
        console_task();
#endif

#if (RP_CORE1_START != TRUE)
#ifdef QUANTUM_PAINTER_ENABLE
        // Run Quantum Painter task
        void qp_internal_task(void);
        qp_internal_task();
        display_task_user();
#endif
#endif

#ifdef DEFERRED_EXEC_ENABLE
        // Run deferred executions
        void deferred_exec_task(void);
        deferred_exec_task();
#endif // DEFERRED_EXEC_ENABLE

        housekeeping_task();
    }
}



void suspend_power_down_user(void)
{
    // code will run multiple times while keyboard is suspended
    // qp_stop_animation(my_anim);
    // LCD Power OFF， Backlight OFF
    palSetLine(17U);
}

void suspend_wakeup_init_user(void)
{
    // code will run on keyboard wakeup
    // Enable Power
    palClearLine(17U);
    // start_gif();
}