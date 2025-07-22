#include "qp.h"
#include "qp_comms.h"

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
uint8_t gif_playing_id = 1;
painter_font_handle_t my_font;
painter_image_handle_t playing_gif;
static uint8_t boot_displaying = 1;


/* rgb info */
extern rgblight_config_t rgblight_config;
extern uint16_t kb_idle_timer;
extern uint8_t indicator_state;

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
    wait_ms(300);
    palClearLine(17U); //power on and wait
    wait_ms(300);

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

    // boot gif
    #ifndef BOOTGIF
    playing_gif = qp_load_image_mem(gfx_boot);
    #else
    playing_gif = qp_load_image_mem(BOOTGIF);
    #endif

    kb_idle_timer = 0;
    gif_started = 0;
    
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
        }
        return;
    }
    static uint8_t prev_gif_id = 99; 
    static painter_image_handle_t logo_image;
    // 0 for caps; 1 for typing; they are both 1M max. Other 4 gifs are 2M max.
    static const uint32_t gif_addr[6] = { (0x1040<<16), (0x1050<<16), (0x1060<<16), (0x1080<<16), (0x10A0<<16), (0x10C0<<16)};

    // capslock
    if (indicator_state & 1) {
        if (gif_playing_id != 0) {
            gif_playing_id = 0;
            qp_stop_animation(my_anim);
            qp_close_image(playing_gif);
            playing_gif = qp_load_image_mem(gif_addr[gif_playing_id]);
            gif_started = 0;
        }
    }

    else if (prev_gif_id != gif_playing_id) {
        if (gif_playing_id == 0) gif_playing_id = prev_gif_id;
        else if (gif_playing_id > 5) gif_playing_id = 1;
        qp_stop_animation(my_anim);
        qp_close_image(playing_gif);
        playing_gif = qp_load_image_mem(gif_addr[gif_playing_id]);

        if (gif_playing_id != prev_gif_id) {
            qp_rect(display, 0, 0, LCD_HEIGHT, LCD_WIDTH, 0, 0, 0, 1); //default black
            char gif_num[10] = {};
            sprintf(gif_num, "GIF %d", gif_playing_id);
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
        prev_gif_id = gif_playing_id;
    }
}

void display_task_user(void)
{
    update_gif_task();
    if (kb_idle_timer >= 1 && gif_playing_id == 1) {
        qp_stop_animation_frame(my_anim);
    } else if (gif_started == 0) {
        qp_stop_animation(my_anim);
        my_anim = qp_animate(display, 0, 0, playing_gif);
        gif_started = 1;
    }
}