#include "qp.h"
#include "qp_comms.h"
#include "c1.h"

#include "qp_gc9xxx_opcodes.h"
#include "qp_gc9107_opcodes.h"
#include "gfx/boot.qgf.h"
#include "gfx/boot2.qgf.h"

#include "gfx/robotomono20.qff.h"

#include "color.h"
#include "config.h"
#include "timer.h"

bool is_st7735 = false;
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


void keyboard_pre_init_kb (void) {
    user_eeconfig.raw = eeconfig_read_user();
    is_st7735 = user_eeconfig.is_st7735;
}

bool qp_gc9107_init(painter_device_t device, painter_rotation_t rotation) {
    // A lot of these "unknown" opcodes are sourced from other OSS projects and are seemingly required for this display to function.
    // clang-format off
    const uint8_t gc9107_init_sequence[] = {
        GC9XXX_SET_INTER_REG_ENABLE1,   5,  0,
        GC9XXX_SET_INTER_REG_ENABLE2,   5,  0,
        GC9107_SET_FUNCTION_CTL6, 0, 1, GC9107_ALLOW_SET_COMPLEMENT_RGB | 0x08 | GC9107_ALLOW_SET_FRAMERATE,
        GC9107_SET_COMPLEMENT_RGB, 0, 1, GC9107_COMPLEMENT_WITH_LSB,
        0xAB, 0, 1, 0x0E,
        GC9107_SET_FRAME_RATE, 0, 1, 0x19,
        GC9XXX_SET_PIXEL_FORMAT, 0, 1, GC9107_PIXEL_FORMAT_16_BPP_IFPF,
        GC9XXX_CMD_SLEEP_OFF,   120, 0,
        GC9XXX_CMD_DISPLAY_ON,  20,  0
    };

    const uint8_t gc9107_init_sequence_st7735_fix[] = {
        GC9XXX_CMD_INVERT_ON,   20, 0 
    };

    // Configure the rotation (i.e. the ordering and direction of memory writes in GRAM)
    const uint8_t madctl[] = {
        [QP_ROTATION_0]   = GC9XXX_MADCTL_BGR,
        [QP_ROTATION_90]  = GC9XXX_MADCTL_BGR | GC9XXX_MADCTL_MX | GC9XXX_MADCTL_MV,
        [QP_ROTATION_180] = GC9XXX_MADCTL_BGR | GC9XXX_MADCTL_MX | GC9XXX_MADCTL_MY,
        [QP_ROTATION_270] = GC9XXX_MADCTL_BGR | GC9XXX_MADCTL_MV | GC9XXX_MADCTL_MY,
    };

    qp_comms_bulk_command_sequence(device, gc9107_init_sequence, sizeof(gc9107_init_sequence));

    if (is_st7735) qp_comms_command_databyte(device, GC9XXX_CMD_INVERT_ON, 0); 
    
    if (is_st7735) {
        qp_comms_command_databyte(device, GC9XXX_SET_MEM_ACS_CTL, madctl[QP_ROTATION_0]);
    } else {
        qp_comms_command_databyte(device, GC9XXX_SET_MEM_ACS_CTL, madctl[rotation]);
    }

    return true;
}

void display_is_st7735_toggle(void) {
    user_eeconfig.is_st7735 ^= 1;
    eeconfig_update_user(user_eeconfig.raw);
    is_st7735 = user_eeconfig.is_st7735;
}

void display_power_toggle(void) {
    user_eeconfig.lcd_off ^= 1;
    eeconfig_update_user(user_eeconfig.raw);
    now_lcd_off = user_eeconfig.lcd_off;
    if (now_lcd_off) {
        qp_stop_animation(my_anim);
        prev_gif_id = 99;
        palSetLine(17U); //power off
    } else {
        palClearLine(17U); //power on
    }
}

void next_gif_id(void) {
    now_gif_id++;
    if (now_gif_id > 5) now_gif_id = 1;
    user_eeconfig.gif_id = now_gif_id;
    eeconfig_update_user(user_eeconfig.raw);
}

//user config end

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
                gif_started = 0;
            }
            return;
        }
    }
}

void display_init(void)
{
    // LCD Power
    palSetLineMode(17U, PAL_MODE_OUTPUT_PUSHPULL | PAL_RP_PAD_DRIVE12);
    palSetLine(17U); //power off to reset the lcd
    wait_ms(1000);
    palClearLine(17U); //power on and wait
    wait_ms(200);

    // Display Init
    display = qp_gc9107_make_spi_device(LCD_HEIGHT, LCD_WIDTH, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, LCD_SPI_DIVISOR, SPI_MODE);
    qp_init(display, LCD_ROTATION);

    // Display offset
    qp_set_viewport_offsets(display, LCD_OFFSET_X, LCD_OFFSET_Y);

    // Power on display, RGB Test
    // qp_rect(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint8_t hue, uint8_t sat, uint8_t val, bool filled);
    qp_power(display, 1);
    qp_rect(display, 0, 0, LCD_HEIGHT, LCD_WIDTH, 0, 0, 0, 1); //default black
    // font
    my_font = qp_load_font_mem(font_robotomono20);

    #if 0
    // testing
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
    playing_gif = qp_load_image_mem(gfx_boot);
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
    if (boot_displaying) {
        kb_idle_timer = 0;

        if (boot_displaying == 1 && animation_states[0].frame_number > 1) {
            boot_displaying = 2;
        }

        if (boot_displaying == 2 && animation_states[0].frame_number == 1) {
            boot_displaying = 0;
            wait_ms(800);   
            qp_rect(display, 0, 0, LCD_HEIGHT, LCD_WIDTH, 0, 0, 0, 1); //default black
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
    // 0 for caps; 1 for typing; they are both 1M max. Other 4 gifs are 2M max.
    static const uint32_t gif_addr[6] = { (0x1040<<16), (0x1050<<16), (0x1060<<16), (0x1080<<16), (0x10A0<<16), (0x10C0<<16)};

    // capslock
    if (indicator_state & 1) {
        if (now_gif_id != 0) {
            now_gif_id = 0;
            qp_stop_animation(my_anim);
            qp_close_image(playing_gif);
            playing_gif = qp_load_image_mem(gif_addr[now_gif_id]);
            gif_started = 0;
        }
    }

    else if (prev_gif_id != now_gif_id) {
        if (now_gif_id == 0) now_gif_id = (prev_gif_id > 10)?1:prev_gif_id;
        else if (now_gif_id > 5) now_gif_id = 1;
        qp_stop_animation(my_anim);
        qp_close_image(playing_gif);
        playing_gif = qp_load_image_mem(gif_addr[now_gif_id]);

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
        kb_idle_timer = 0;
        gif_started = 0;
        //save prev
        prev_gif_id = now_gif_id;
    }
}

void display_task_user(void)
{
    if (!boot_displaying && user_eeconfig.lcd_off) return;
    //return;
    update_gif_task();

    if (!boot_displaying && kb_idle_timer == 0 && now_gif_id == 1) { 
        static uint8_t prev_frame = 0;
        if (animation_states[0].frame_number != prev_frame) {
            animation_states[0].frame_number += 2;
            if (animation_states[0].frame_number >= animation_states[0].image->frame_count) {
                animation_states[0].frame_number = 0;
            }
            prev_frame = animation_states[0].frame_number;
        }
    } else if (gif_started == 0) {
        qp_stop_animation(my_anim);
        my_anim = qp_animate(display, 0, 0, playing_gif);
        gif_started = 1;
    }
#if 0
    if (kb_idle_timer >= 1 && now_gif_id == 1) {
        qp_stop_animation_frame(my_anim);
    } else if (gif_started == 0) {
        qp_stop_animation(my_anim);
        my_anim = qp_animate(display, 0, 0, playing_gif);
        gif_started = 1;
    }
#endif
}

void suspend_power_down_user_display(void)
{
    // keep power off
    // LCD Power OFF， Backlight OFF
    if (!now_lcd_off) {
        now_lcd_off = 1;
        if (!boot_displaying) qp_stop_animation(my_anim);
        prev_gif_id = 99;
        palSetLine(17U);
    }
}

void suspend_wakeup_init_user_display(void)
{
    if (now_lcd_off && (!user_eeconfig.lcd_off || boot_displaying)) {
        // Enable Power
        palClearLine(17U);
        wait_ms(200);
        now_lcd_off = 0;
    }
}
