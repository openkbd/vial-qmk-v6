#ifndef SWITCH_BOARD_H
#define SWITCH_BOARD_H

#include <stdint.h>
#include <stdbool.h>

extern bool is_ver5020;

#define GPIO_OUTPUT_MODE (PAL_MODE_OUTPUT_PUSHPULL)
#define GPIO_INPUT_MODE (PAL_MODE_INPUT_PULLUP)

static inline void KEY_SDI_OFF(void) {
    if (is_ver5020) {
        palClearLine(7U);
    } else {
        palSetLine(7U);
    }
}
static inline void KEY_SDI_ON(void) {
    if (is_ver5020) {
        palSetLine(7U);
    } else {
        palClearLine(7U);
    }
}

static inline void get_key_ready(void) {
    palSetLine(7U);
    palSetLineMode(7U, GPIO_INPUT_MODE);
    wait_us(4);
}

static inline void select_key_ready(void) {
    palSetLineMode(7U, GPIO_OUTPUT_MODE);
} 

//SCK
#define CLOCK_PULSE() \
    do { \
        palSetLine(6U); asm("nop");\
        palClearLine(6U); \
    } while(0)


#endif