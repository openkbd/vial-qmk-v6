#pragma once
// override backing_store_lock/unlock to control core1
#pragma weak backing_store_lock
#pragma weak backing_store_unlock

/* USB Device descriptor parameter */
#undef  PRODUCT
#define PRODUCT         "Athena1800 Keybaord (VIAL_DQ69)"

#define EECONFIG_KB_DATA_SIZE 4
#define VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT (1<<6 | 2<<3 | 6)
#define RGBLIGHT_DEFAULT_MODE 8

/* key matrix size */
#define MATRIX_ROWS 14 //max supported
#define MATRIX_COLS 8
#define SOFTWARE_ESC_BOOTLOADER
#define WS2812_CALL_DRIVER_PREV

#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U

#undef  CRT0_EXTRA_CORES_NUMBER
#define CRT0_EXTRA_CORES_NUMBER 1

/* SPI pins */
#define SPI_DRIVER SPID1
#define SPI_SCK_PIN GP14
#define SPI_MOSI_PIN GP15
#define SPI_MISO_PIN GP20 // Unused

/* LCD Configuration */
#define LCD_RST_PIN GP16
#define LCD_DC_PIN GP12
#define LCD_CS_PIN GP13
#define LCD_BLK_PIN GP7 // Unused in this configuration
#define LCD_SPI_DIVISOR 4
#define LCD_ROTATION QP_ROTATION_0
//#define LCD_ROTATION QP_ROTATION_180
// OFFSET 2,1 for 0.85 GC9701 TFT
#define LCD_OFFSET_X 2
#define LCD_OFFSET_Y 1
//#define LCD_INVERT_COLOR
#define LCD_WIDTH 128
#define LCD_HEIGHT 128
#define QUANTUM_PAINTER_TASK_THROTTLE 1
#define QUANTUM_PAINTER_SUPPORTS_NATIVE_COLORS TRUE
#define QUANTUM_PAINTER_SUPPORTS_256_PALETTE TRUE
#define SPI_MODE 0
#define GC_9107

// QP Configuration
#define QUANTUM_PAINTER_SUPPORTS_NATIVE_COLORS TRUE
#define QUANTUM_PAINTER_SUPPORTS_256_PALETTE TRUE
#define QUANTUM_PAINTER_CONCURRENT_ANIMATIONS 1
//#define ST7789_NO_AUTOMATIC_VIEWPORT_OFFSETS

// Timeout configuration, default 30000 (30 sek). 0 = No timeout. Beware of image retention.
#define QUANTUM_PAINTER_DISPLAY_TIMEOUT 0

// Mouse Key
#define MOUSEKEY_MOVE_DELTA 2

/* key combination for command */
#define IS_COMMAND() ( \
    (get_mods() == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT))) || \
    (get_mods() == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_LCTRL) | MOD_BIT(KC_RSHIFT))) \
)
