/*
Copyright 2025 YANG

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
#include "quantum.h"
#include "via.h"
#include "hardware/watchdog.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
//#include "pico/multicore.h"

#ifndef LED_TYPE
#define LED_TYPE rgb_led_t
#endif

extern uint8_t indicator_color_config[];
extern LED_TYPE indicator_color[];

void rprint(char *msg) {
    return;
    //0xfdee
    uint8_t eeee_buf[32] = {0};
    uint8_t msg_len = strlen(msg);
    if (msg_len > 30) msg_len = 30; //一次发送最大长度限制，长了可以分多次发送
    memcpy(&eeee_buf[2], msg, msg_len);
    eeee_buf[0] = 0xFD;
    eeee_buf[1] = 0xEE;
    raw_hid_send(eeee_buf, 32);
}

void raw_hid_send_bouncing_key(uint8_t row, uint8_t col) {
    return; //直接发送额外命令，会导致via出现错误信息，vial在matrix tester的时候也会显示出错。
    //0xfdbc
    uint8_t buf[32] = {0};
    buf[0] = 0xFD;
    buf[1] = 0xBC;
    buf[2] = row;
    buf[3] = col;
    raw_hid_send(buf, 32);
}


static void call_flash_range_program(void *param) {
    uint32_t offset = ((uintptr_t*)param)[0];
    const uint8_t *data = (const uint8_t *)((uintptr_t*)param)[1];
    flash_range_program(offset, data, 256);
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &(data[0]);
    static uint8_t page_data[256] = {0};
    if (*command_id == 0xFD) {
        if (data[1] == 0xF1) {
            // 0xF1: 用于写入
            /* FD FC AD DR OF + 数据, 前面占了5个，每次能传的数据就是32-5 = 27
                地址根据VIA的习惯，高位在前。
               */
            if (data[4] == 0) {
                //data start
                memset(page_data, 0, sizeof(page_data));
            }
            for (uint8_t i=0; i<27; i++) {
                uint16_t target = data[4] + i;
                if (target < 256) {
                    page_data[target] = data[5+i];
                } else {
                    // 有256时，写入
                    uint32_t offset = (data[2] << 16) | (data[3] << 8);
                    if (offset < 0x400000) return;
                    #if 1
                    xprintf("\n Write 256B to 0x%06X :\n",offset);
                    for (uint8_t y=0; y<16; y++) {
                        for (uint8_t x=0; x<16; x++) {
                            xprintf("%02X ", page_data[y*16+x]);
                        }
                        print("\n");
                    }
                    #endif
                  #if 1 //目前，双核运行，写入时，会死机
                    //multicore_reset_core1();
                    uint32_t ints = save_and_disable_interrupts();
                    flash_range_erase(0xE00000, 4096);
                    //flash_range_program(0xE00000, page_data, 256);
                    restore_interrupts (ints);
                  #endif
                  #if 1 //写入后，读取验证
                    static const uint8_t *flash_target = (const uint8_t *)0x10E00000;
                    xprintf("\n Read Test Addr1 0x%X: ", flash_target);
                    for (uint8_t y=0; y<16; y++) {
                        for (uint8_t x=0; x<16; x++) {
                            xprintf("%02X ", flash_target[y*16+x]);
                        }
                        print("\n");
                    }
                  #endif
                    break;
                }
            }
        }
    }
}

//after set layout command
void via_set_layout_options_after(void)
{
    //layout 选项里有用来存储配置的。
    user_eeconfig_init();
}

#define DEBOUNCE_DN(x) (uint8_t)(~(0x80 >> x))
#define DEBOUNCE_UP(x) (uint8_t)(0x80 >> x)
extern uint8_t now_debounce_dn_mask;
extern uint8_t now_debounce_up_mask;
static debounce_dn_level[3] = {DEBOUNCE_DN(1), DEBOUNCE_DN(3), DEBOUNCE_DN(6)};
static debounce_up_level[3] = {DEBOUNCE_UP(4), DEBOUNCE_UP(5), DEBOUNCE_UP(7)};

void update_debounce_level(level) {
    level = level & 0b11;
    now_debounce_dn_mask = debounce_dn_level[level];
    now_debounce_up_mask = debounce_up_level[level];
    xprintf("\n debounce dn: %08b, up:%08b", now_debounce_dn_mask, now_debounce_up_mask);
}

void user_eeconfig_init(void)
{
    // 0 红，21 橙， 42 黄， 85 绿，127 青，170 蓝，212紫，
    // 对应是360度的HSL值，是 0度，30度，60度，120度，180度，240度，300度。
    // 最后的255用于后面再针对处理，用于白色。
    static const uint8_t indicator_hue_preset[8] = {0, 21, 42, 85, 127, 170, 212, 255};
    #ifdef INDICATOR_VAL
    static uint8_t val = INDICATOR_VAL;
    #else 
    static uint8_t val = 255;
    #endif
     //使用自带的命令读取，不用手动换顺序，否则写入时是先高位后低位。
     //比如需要是0xAABB，16位直接读取结果会是0xBBAA
    uint16_t layout_value = via_get_layout_options();
    for (uint8_t i=0; i<3; i++) {
        indicator_color_config[i] = (layout_value & 0b111);
        uint8_t hue = indicator_hue_preset[ indicator_color_config[i] ];
        layout_value >>= 3;
        if (hue == 255) indicator_color[i] = (LED_TYPE){0, 0, 0};
        else            indicator_color[i] = hsv_to_rgb((HSV){hue, 255, val});
        if (i < 2) xprintf("\n indicator %d R: %d, G: %d, B:%d", i, indicator_color[i].r, indicator_color[i].g, indicator_color[i].b);
    }
    update_debounce_level(indicator_color_config[2]);
    led_wakeup(); //立即更新指示灯颜色
    rprint("Layout set change\n");
}

