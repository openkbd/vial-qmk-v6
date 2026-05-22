// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

void c1_main_task(void);
void c1_before_flash_operation(void);
void c1_after_flash_operation(void);
bool lcd_is_on(void);

void suspend_power_down_user_display(void);
void suspend_wakeup_init_user_display(void);

/* user config saved in eeprom */
typedef union {
    uint32_t raw;
    struct {
        uint8_t  gif_id  :      4;
        bool     lcd_off :      1;
        bool     is_st7735 :    1;
    };
} user_eeconfig_t;