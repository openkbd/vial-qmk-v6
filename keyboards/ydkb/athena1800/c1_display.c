#include "qp.h"
#include "qp_comms.h"
#include "c1.h"

#include "qp_gc9107_opcodes.h"
#include "gfx/boot.qgf.h"
#include "gfx/boot2.qgf.h"

#include "gfx/robotomono20.qff.h"

#include "color.h"
#include "config.h"
#include "timer.h"

painter_device_t display;
static deferred_token my_anim;
static bool gif_started = 0;
static uint8_t prev_gif_id = 99;
static uint8_t now_gif_id = 1;
static bool now_lcd_off = 0;

painter_font_handle_t my_font;
painter_image_handle_t playing_gif;
static uint8_t boot_displaying = 1;


/* rgb info */
extern rgblight_config_t rgblight_config;
extern uint16_t kb_idle_timer;
extern uint8_t indicator_state;


user_eeconfig_t user_eeconfig;

void display_power_toggle(void) {
    user_eeconfig.lcd_off ^= 1;
    eeconfig_update_user(user_eeconfig.raw);
    now_lcd_off = user_eeconfig.lcd_off;
    if (now_lcd_off) {
        qp_stop_animation(my_anim); //停止动画，防止下次再打开时，动画飞速
        prev_gif_id = 99; //设置为99，以便下次恢复
        palSetLine(17U); //power off
    } else {
        palClearLine(17U); //power on
    }
}

void next_gif_id(void) {
    now_gif_id++;
    if (now_gif_id > 5) now_gif_id = 1;
    user_eeconfig.gif_id = now_gif_id;
    eeconfig_update_user(user_eeconfig.raw); //保存
}

//user config end

/* 从 qp_draw_image.c 复制过来，重新定义, 让gif停止播放时，停在指定的帧(这里是第1帧) */
typedef struct animation_state_t {
    painter_device_t       device;
    uint16_t               x;
    uint16_t               y;
    painter_image_handle_t image;
    qp_pixel_t             fg_hsv888;
    qp_pixel_t             bg_hsv888;
    uint16_t               frame_number;
    deferred_token         defer_token;
} animation_state_t;

extern deferred_executor_t animation_executors[QUANTUM_PAINTER_CONCURRENT_ANIMATIONS];
extern animation_state_t   animation_states[QUANTUM_PAINTER_CONCURRENT_ANIMATIONS];

void qp_stop_animation_frame(deferred_token anim_token) {
    for (int i = 0; i < QUANTUM_PAINTER_CONCURRENT_ANIMATIONS; ++i) {
        if (animation_states[i].defer_token == anim_token) {
            if (animation_states[i].device != NULL && animation_states[i].frame_number == 1) {
                cancel_deferred_exec_advanced(animation_executors, QUANTUM_PAINTER_CONCURRENT_ANIMATIONS, anim_token);
                animation_states[i].device = NULL;
                gif_started = 0; //直到停止动画后，才设置这个值。
            }
            return;
        }
    }
}
//-----------------------end-----------------

void display_init(void)
{
    // LCD Power
    palSetLineMode(17U, PAL_MODE_OUTPUT_PUSHPULL | PAL_RP_PAD_DRIVE12);
    palSetLine(17U); //power off to reset the lcd，有可能并未reset
    wait_ms(1000);
    palClearLine(17U); //power on and wait
    wait_ms(200); //缺少了这一条，gif不动。双核似乎去掉也没问题。

    // Display Init
    display = qp_gc9107_make_spi_device(LCD_HEIGHT, LCD_WIDTH, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, LCD_SPI_DIVISOR, SPI_MODE);
    qp_init(display, LCD_ROTATION);

    // Display offset
    qp_set_viewport_offsets(display, LCD_OFFSET_X, LCD_OFFSET_Y);

    // Power on display, RGB Test
    // qp_rect(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint8_t hue, uint8_t sat, uint8_t val, bool filled);
    qp_power(display, 1);
    qp_rect(display, 0, 0, LCD_HEIGHT, LCD_WIDTH, 0, 0, 0, 1); //default black
    // 载入字体
    my_font = qp_load_font_mem(font_robotomono20);

    // 显示启动信息，这里之后使用一个gif来代替。
    #if 0
    if (my_font != NULL) {
        qp_drawtext(display, 0, 0, my_font, "Hello");
        qp_drawtext(display, 0, 30, my_font, "From");
        qp_drawtext(display, 0, 60, my_font, "QMK!");
        wait_ms(200);
        qp_drawtext(display, 0, 90, my_font, "Booting");
        wait_ms(200);
        qp_drawtext(display, 0, 90, my_font, "Booting.");
        wait_ms(200);
        qp_drawtext(display, 0, 90, my_font, "Booting..");
        wait_ms(200);
        qp_drawtext(display, 0, 90, my_font, "Booting...");
    }
    #endif
    // boot gif
    #ifndef BOOTGIF
    playing_gif = qp_load_image_mem(gfx_boot); //内置的启动动画
    #else
    playing_gif = qp_load_image_mem(BOOTGIF);
    #endif

    kb_idle_timer = 0;
    gif_started = 0;

}
bool lcd_is_on(void)
{
    return (boot_displaying || (now_lcd_off == 0));
}

void update_gif_task(void) {
    //启动动画播放一次
    if (boot_displaying) {
        kb_idle_timer = 0;
        // 开始播放
        if (boot_displaying == 1 && animation_states[0].frame_number > 1) {
            boot_displaying = 2;
        }
        // 已完成一次循环
        if (boot_displaying == 2 && animation_states[0].frame_number == 1) {
            boot_displaying = 0;
            wait_ms(800);

            //after boot gif, poweroff if lcd is disabled
            if (user_eeconfig.lcd_off) {
                palSetLine(17U); //power off to reset the lcd
            }
            //完成播放后，初始化部分数据
            now_gif_id = user_eeconfig.gif_id;
            now_lcd_off = user_eeconfig.lcd_off;
        }
        return;
    } else if (now_lcd_off) {
        return;
    }

    static painter_image_handle_t logo_image;
    // 0 是caps动画，1是打字动画，这两个限制大小不超过1M，其他的最大为2M。
    static const uint32_t gif_addr[6] = { (0x1040<<16), (0x1050<<16), (0x1060<<16), (0x1080<<16), (0x10A0<<16), (0x10C0<<16)};

    // capslock 打开时，切换显示
    if (indicator_state & 1) {
        if (now_gif_id != 0) {
            now_gif_id = 0;
            qp_stop_animation(my_anim); // always stop before qp_load
            qp_close_image(playing_gif); // Unload,如果不unload，循环载入多次后会出错。
            playing_gif = qp_load_image_mem(gif_addr[now_gif_id]);
            gif_started = 0;
        }
    }
    //now_gif_id 作为全局变量，变更后，才更换
    else if (prev_gif_id != now_gif_id) {
        if (now_gif_id == 0) now_gif_id = (prev_gif_id > 10)?1:prev_gif_id; //防止一开始capslock就打开时，显示动画为caps的，再关闭caps时，id恢复不正常。
        else if (now_gif_id > 5) now_gif_id = 1;
        qp_stop_animation(my_anim);
        qp_close_image(playing_gif);
        playing_gif = qp_load_image_mem(gif_addr[now_gif_id]);

        // 仅在变更时才显示一次序号
        if (now_gif_id != prev_gif_id) {
            qp_rect(display, 0, 0, LCD_HEIGHT, LCD_WIDTH, 0, 0, 0, 1); //default black
            char gif_num[10] = {};
            sprintf(gif_num, "GIF %d", now_gif_id);
            qp_drawtext(display, 0, 0, my_font, gif_num);
            if (playing_gif->width == 128 && playing_gif->width == 128) {
                wait_ms(100);
            } else {
                qp_drawtext(display, 0, 30, my_font, "To be");
                qp_drawtext(display, 0, 60, my_font, "uploaded.");
            }
        }
        kb_idle_timer = 0; //避免用其他键盘切换caps时，gif未更新。
        gif_started = 0;
        //save prev
        prev_gif_id = now_gif_id;
    #if 0 //用于测试读取gif数据，以确认是否正常。
        static const uint8_t *flash_target = (const uint8_t *)0x10400000;
        xprintf("Read Test Addr1 0x%X: ", flash_target);
        for (uint8_t i=0; i<16; i++) {
            xprintf("%02X ", flash_target[i]);
        }
        print("\n");
    #endif
    }
}

void display_task_user(void)
{
    if (!boot_displaying && user_eeconfig.lcd_off) return;
    //return;

    update_gif_task();

    if (!boot_displaying && kb_idle_timer == 0 && now_gif_id == 1) { // 播放gif时，约2ms一次，
        static uint8_t prev_frame = 0;
        if (animation_states[0].frame_number != prev_frame) {
            animation_states[0].frame_number += 2;
            if (animation_states[0].frame_number >= animation_states[0].image->frame_count) {
                animation_states[0].frame_number = 0;
            }
            prev_frame = animation_states[0].frame_number;
        }
        //animation_states[0].frame_number += 1;
    } else if (gif_started == 0) {
        // 只在此处开始播放，播放前先停止当前的。
        qp_stop_animation(my_anim);
        my_anim = qp_animate(display, 0, 0, playing_gif);
        gif_started = 1;
    }
#if 0
    if (kb_idle_timer >= 1 && now_gif_id == 1) { // 播放gif时，约2ms一次，
        qp_stop_animation_frame(my_anim);
    } else if (gif_started == 0) {
        // 只在此处开始播放，播放前先停止当前的。
        qp_stop_animation(my_anim);
        my_anim = qp_animate(display, 0, 0, playing_gif);
        gif_started = 1;
    }
#endif
#if 0
    static uint32_t last_draw = 0;
    if (timer_elapsed32(last_draw) > 33) { // Throttle to 30fps
        last_draw = timer_read32();
        static int i = 0;
        // Draw r=4 filled circles down the left side of the display
        //for (int i = 0; i < 127; i+=8) {
            qp_circle(display, 4, 4+i, 4, i, 255, 255, true);
        //}
        i += 8;
        if (i >= 127) i = 0;
        qp_flush(display);
    }
#endif
}

void suspend_power_down_user_display(void)
{
    // keep power off
    // LCD Power OFF， Backlight OFF
    if (!now_lcd_off) {
        now_lcd_off = 1;
        qp_stop_animation(my_anim); //停止动画，防止下次再打开时，动画飞速
        prev_gif_id = 99; //设置为99，以便下次恢复
        palSetLine(17U);
    }
}

void suspend_wakeup_init_user_display(void)
{
    // 1 && 0时，肯定是从休眠唤醒。
    if (now_lcd_off && !user_eeconfig.lcd_off) {
        // Enable Power
        palClearLine(17U);
        wait_ms(200);
        now_lcd_off = 0;
    }
}
