#ifndef SWITCH_BOARD_H
#define SWITCH_BOARD_H

#include <stdint.h>
#include <stdbool.h>


#define GPIO_OUTPUT_MODE (PAL_MODE_OUTPUT_PUSHPULL)
#define GPIO_INPUT_MODE (PAL_MODE_INPUT_PULLUP)

//SDI 
static inline void KEY_SDI_OFF(void) {
    palClearLine(7U);
}
static inline void KEY_SDI_ON(void) {
    palSetLine(7U);
}

static inline void get_key_ready(void) {
    /*  不使用这一条时，如果key无上拉，30ms时按住多个按键时会触发不相关的。
        比如按住ASDF，R也被触发了。*/
    palSetLine(7U);
    wait_us(2);
    palSetLineMode(7U, GPIO_INPUT_MODE);
    // 目前30us，在RP2040上，才不会同时触发同个按键
    //wait_us(30);
    // 如果将key 上拉10K，10us。使用2.2K的时候，5us有按键会无反应。
    // 使用4.7K时，用3us同时按住多个按键时会触发不相关的，使用4us时无问题。保险一点可以用5us
    // 4us 时，有的轴体一直快速交替按多个按键，会严重抖动。
    // 用owl的gif，树枝上走来走去那个。6us时速度是550，8us时是542。速度区别不是很大。这个时间不准，要把matrix_scan()里仅每ms扫描的代码去了，才增加一些到800多。
    // 可以把wait_us()分一点在palSetLine(7U)那里。稳定按键，这两个延迟一共需要6us到8us。
    // 6us 80x，7us 69x。
    wait_us(4);
}

static inline void select_key_ready(void) {
    palSetLineMode(7U, GPIO_OUTPUT_MODE);
} 

//SCK 
//asm("nop");
//RP2040 + 5020轴板，需要这个nop
//RP2040 一个nop 是8ns
#define CLOCK_PULSE() \
    do { \
        palSetLine(6U); asm("nop");\
        palClearLine(6U); \
    } while(0)

//RCK PB11
#if 0
#define KEYS_LATCH() \
    do { \
        palSetLine(5U); asm("nop");\
        palClearLine(5U); \
    } while(0)

#endif
#endif