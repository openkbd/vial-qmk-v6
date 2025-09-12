/*
Copyright 2023 YANG <drk@live.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include "ch.h"
#include "led.h"
#include "rgblight.h"

#include "stdint.h"
#include "quantum.h"

#ifndef LOGIC_INDICATOR_NUM
#define LOGIC_INDICATOR_NUM PHY_INDICATOR_NUM
#endif

#ifndef LED_TYPE
#define LED_TYPE rgb_led_t
#endif

extern rgblight_config_t rgblight_config;

static LED_TYPE RGBLIGHT_COLOR_OFF = { .r = 0, .g = 0, .b = 0 };
uint8_t indicator_state = 0;
//默认保留3组LED灯设置，这个设置也可能用于其他作用，比如灯条同步CapsLock指示灯。
uint8_t indicator_color_config[3];
LED_TYPE indicator_color[3];


LED_TYPE rgbled[PHY_INDICATOR_NUM+RGBLED_NUM];
extern uint8_t rgbinfo_display_on;

void set_rgb_user(uint8_t r, uint8_t g,  uint8_t b)
{
    for (uint8_t i=0; i<(PHY_INDICATOR_NUM+RGBLED_NUM); i++) {
        rgbled[i].r = r;
        rgbled[i].g = g;
        rgbled[i].b = b;
    }
    ws2812_setleds(rgbled, PHY_INDICATOR_NUM+RGBLED_NUM);
}

void rgblight_user_init(void)
{
#ifdef CONFIG_BOOT_TEST_RGB
    //test all leds
    set_rgb_user(32, 0, 0);
    wait_ms(300);
    set_rgb_user(0, 32, 0);
    wait_ms(300);
    set_rgb_user(0, 0, 32);
    wait_ms(300);
    set_rgb_user(0, 0, 0);
#else
    // all leds off
    set_rgb_user(0, 0, 0);
#endif
}

void rgblight_call_driver(LED_TYPE *start_led, uint8_t num_leds) {
    // keep indicator color
    for (uint8_t i=0; i<PHY_INDICATOR_NUM; i++) {
        if (indicator_state & (1<<i)) {
            rgbled[i] = indicator_color[i];
        } else {
            rgbled[i] = RGBLIGHT_COLOR_OFF;
        }
    }


    memcpy(&rgbled[PHY_INDICATOR_NUM], start_led, RGBLED_NUM*3);
#ifdef RGB_EXTRA_PROCESS_ENABLE
    rgb_extra_process(rgbled);
#endif

    ws2812_setleds(rgbled, PHY_INDICATOR_NUM+RGBLED_NUM);
}

extern bool bootmagic_checked;

//new qmk 是使用下面这一个调用设置了。
bool led_update_user(led_t usb_led) {
    led_set_user(usb_led);
}

void led_set_user(uint8_t usb_led)
{
    indicator_state = 0;
#ifdef INDICATOR_FUNCT
    static uint8_t indicator_funct[LOGIC_INDICATOR_NUM] = INDICATOR_FUNCT;
    for (uint8_t i=0; i<LOGIC_INDICATOR_NUM; i++) {
        if (usb_led & indicator_funct[i]) {
            indicator_state |= (1<<i);
        }
    }
    // 固定颜色模式下，更新一次。否则在关闭led时，指示灯颜色未更新。
    if (rgblight_config.mode == 1) rgblight_mode_noeeprom(rgblight_config.mode);
    // 250714 如果不加 if (bootmagic_checked), 在新键盘(或者旧键盘清空Flash或还原eeprom后)，会卡启动。
    if (bootmagic_checked) rgblight_set(); //set rgb even when rgblight.enable=0
#endif
}
void hook_keyboard_loop(void)
{
    //在bootmagic执行完后，再初始化RGB。否则1是不符合逻辑，2是会卡启动导致键盘死机不可用。
    static uint8_t rgb_inited = 0;
    if (rgb_inited == 0 && bootmagic_checked) { 
        //读取指示灯颜色设置
        user_eeconfig_init();
        //运行一次以熄灭掉所有RGB灯。
        rgblight_user_init();
        rgb_inited = 1;
    }
}