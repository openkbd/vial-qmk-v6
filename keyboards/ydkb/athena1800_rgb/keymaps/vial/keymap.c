#include QMK_KEYBOARD_H

#ifndef LED_TYPE
#define LED_TYPE rgb_led_t
#endif

#define USER04 0x7E04
#define USER05 0x7E05
#define USER06 0x7E06
#define MAGIC_TOGGLE_NKRO 0x7013

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  {
    {KC_ESCAPE, KC_GRAVE, KC_1, KC_NO, KC_2, KC_F1, KC_F2, KC_3},
    {KC_LGUI, KC_A, KC_Q, KC_NONUS_BSLASH, KC_LCTRL, KC_LSHIFT, KC_CAPSLOCK, KC_TAB},
    {KC_F3, KC_4, KC_F4, KC_5, KC_F5, KC_F6, KC_6, KC_7},
    {KC_X, KC_R, KC_D, KC_E, KC_LALT, KC_Z, KC_S, KC_W},
    {KC_T, KC_Y, KC_F7, KC_8, KC_U, KC_F8, KC_9, KC_I},
    {KC_N, KC_SPACE, KC_H, KC_B, KC_V, KC_G, KC_C, KC_F},
    {KC_F9, KC_0, KC_O, KC_F10, KC_MINUS, KC_P, KC_EQUAL, KC_LBRACKET},
    {KC_SCOLON, KC_L, KC_DOT, KC_RALT, KC_COMMA, KC_M, KC_K, KC_J},
    {KC_F11, KC_F12, KC_BSPACE, KC_RBRACKET, KC_BSPACE, KC_BSLASH, KC_NO, KC_NO},
    {KC_ENTER, KC_UP, KC_RSHIFT, KC_DOWN, KC_LEFT, LT(1, KC_APPLICATION), KC_SLASH, KC_QUOTE},
    {KC_PGUP, KC_DELETE, KC_NO, KC_HOME, KC_NUMLOCK, KC_KP_7, KC_PSCREEN, KC_KP_SLASH},
    {KC_KP_5, KC_KP_2, KC_NO, KC_KP_4, KC_KP_1, KC_KP_0, KC_RIGHT, KC_PGDOWN},
    {KC_KP_8, KC_SCROLLLOCK, KC_PAUSE, KC_KP_ASTERISK, KC_KP_MINUS, KC_KP_9, KC_KP_PLUS, KC_NO},
    {KC_KP_6, KC_KP_3, KC_KP_ENTER, KC_KP_DOT, KC_NO, KC_NO, KC_NO, KC_NO}
  },
  {
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, KC_MS_L, KC_BTN1, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, RGB_TOG, KC_MS_R, KC_BTN2, KC_TRNS, KC_TRNS, KC_MS_D, KC_MS_U},
    {RGB_MOD, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {RGB_VAD, KC_TRNS, RGB_VAI, RGB_SAD, RGB_HUD, RGB_SAI, KC_CALC, RGB_HUI},
    {KC_TRNS, KC_TRNS, USER05,  KC_TRNS, KC_TRNS, KC_MPLY, USER04,  KC_MPRV},
    {KC_TRNS, KC_TRNS, KC_VOLU, KC_TRNS, KC_VOLD, KC_MUTE, MAGIC_TOGGLE_NKRO,  KC_TRNS},
    {KC_TRNS, USER06,  KC_TRNS, KC_MNXT, KC_TRNS, KC_TRNS, KC_NO, KC_NO},
    {KC_TRNS, KC_PGUP, KC_TRNS, KC_PGDOWN, KC_HOME, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, KC_INSERT, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, KC_TRNS, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_END, KC_TRNS},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO}
  },
};

#ifdef RGB_MATRIX_ENABLE
#define _D(X,Y)  
led_config_t g_led_config = { {
    // Key Matrix to LED Index, 这里按矩阵按键位置，再映射到灯。对应上面keymap的值，和灯的位置，逻辑位置对应
    // 轴灯是位号+2
    {      3,     38,     37, NO_LED,     36,      4,      5,     35},
    {     77,     73,     40, NO_LED,     76,     75,     74,     39},
    {      6,     34,      7,     33,      8,      9,     32,     31},
    {     80,     43,     71,     42,     78,     79,     72,     41},
    {     44,     45,     10,     30,     46,     11,     29,     47},
    {     89,     86,     68,     83,     82,     69,     81,     70},
    {     12,     28,     48,     13,     27,     49,     26,     50},
    {     64,     65,     92,     96,     91,     90,     66,     67},
    {     14,     15, NO_LED,     51,     25,     52, NO_LED, NO_LED},
    {     62,     95,     94,     99,     98,     97,     93,     63},
    {     53,     24, NO_LED,     16,     23,     54,     17,     22},
    {     59,    105, NO_LED,     60,    106,    101,    100,     61},
    {     55,     18,     19,     21,     20,     56,     57, NO_LED},
    {     58,    104,    103,    102, NO_LED, NO_LED, NO_LED, NO_LED}
/* 这是此键盘灯的物理位置
  { 3,      NO_LED,  4,      5,      6,      7,      8,      9,     10,     11,     12,     13,     14,     15, NO_LED,     16,    17,   18,    19 },
  { 38,     37,     36,     35,     34,     33,     32,     31,     30,     29,     28,     27,     26,     25,     24,     23,    22,   21,    20 },
  { 39,     40,     41,     42,     43,     44,     45,     46,     47,     48,     49,     50,     51,     52,     53,     54,    55,   56, NO_LED },
  { 74,     73,     72,     71,     70,     69,     68,     67,     66,     65,     64,     63, NO_LED,     62,     61,     60,    59,   58,    57 },
  { 75,     79,     80,     81,     82,     83,     89,     90,     91,     92,     93, NO_LED,     94,     95, NO_LED,    106,   105,  104, NO_LED },
  { 76,     77,     78,     84,     85,     86,     87,     88,  NO_LED,    96,     97, NO_LED,     98,     99,    100,    101, NO_LED, 102,   103 }
*/
}, {
    // LED Index to Physical Position
    // x = 224 / (NUMBER_OF_COLS - 1) * COL_POSITION
    // y =  64 / (NUMBER_OF_ROWS - 1) * ROW_POSITION
    // 这里是按ws2812的灯的顺序，再对应到按键的物理位置。装饰灯在(4，14)位置，对应{168, 176}
    //glow
    {168,  176}, {171,  176}, {174,  176}, 
    //row 0
    {0, 0}, _D(12,0)  {24, 0}, {36, 0},   {48, 0},   {60, 0},   {72, 0},   {84, 0},   {96, 0},   {108, 0},  {120, 0},  {132, 0},  {144, 0},  _D(156, 0)   {180, 0},    {192, 0},  {204, 0},  {216, 0}, // Row 0
    //row1, right to left
    {216, 44}, {204, 44}, {192, 44}, {180, 44}, {168, 44}, {156, 44}, {144, 44}, {132, 44}, {120, 44}, {108, 44}, {96, 44}, {84, 44}, {72, 44}, {60, 44}, {48, 44}, {36, 44}, {24, 44}, {12, 44}, {0, 44},
    //row2 
    {0, 88},  {12, 88},  {24, 88},  {36, 88},  {48, 88},  {60, 88},  {72, 88},  {84, 88},  {96, 88},  {108, 88}, {120, 88}, {132, 88}, {144, 88}, {156, 88}, {168, 88}, {180, 88}, {192, 88}, {204, 88} _D(216,88),
    //row3, right to left
    {216, 132}, {204, 132}, {192, 132}, {180, 132}, {168, 132}, {156, 132}, _D(144,132) {132, 132}, {120, 132}, {108, 132}, {96, 132}, {84, 132}, {72, 132}, {60, 132}, {48, 132}, {36, 132}, {24, 132}, {12, 132}, {0, 132},
    //row4 and 5, lshift lctrl lgui lalt
    { 0, 176}, {0, 220}, {12, 220}, {24, 220},
    //row 4, z to b
    {12, 176}, {24, 176}, {36, 176}, {48, 176}, {60, 176}, 
    //row 5, space
    {36, 220}, {48, 220}, {60, 220}, {72, 220}, {84, 220},
    //row 4, n to up
    {72, 176}, {84, 176}, {96, 176}, {108, 176}, {120, 176}, _D(132,176) {144, 176}, {156, 176} _D(168, 176),
    //row5,RAlt to PEnter
    _D(96,220) {108, 220}, {120, 220}, _D(132, 220) {144, 220}, {156, 220}, {168, 220}, {180, 220}, _D(192,220)  {204, 220}, {216, 220},
    // Row 4, p3 to p1
    _D(216,176) {204, 176}, {192, 176}, {180, 176}
}, {
    // LED 类型: 4 代表普通按键灯, 3个装饰灯 + 6行19列。
    2, 2, 2,
    4,    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,      4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  4,  4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  4,  4, 4, 4, 
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,    4,  4,  4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,    4, 4,      4, 4, 4, 
    4, 4, 4, 4, 4, 4, 4, 4,    4, 4,    4, 4, 4,   4,    4, 4
} };

extern LED_TYPE indicator_color[3];
extern uint8_t indicator_color_config[];
__attribute__ ((weak))

#define CAPS_LOCK_LED_POS    74
#define NUM_LOCK_LED_POS     23
bool rgb_matrix_indicators_user(void) {
    //如果有禁用部分灯
    if (indicator_color_config[2] & 0b10) { //Swithes Disable
        for (uint8_t i=3; i<107; i++) {
            rgb_matrix_set_color(i, 0, 0, 0); 
        }
    }
    if (indicator_color_config[2] & 0b1) { //Underglow Disable
        for (uint8_t i=0; i<3; i++) {
            rgb_matrix_set_color(i, 0, 0, 0); 
        }
    }

    if (host_keyboard_led_state().caps_lock) {
        //RGB led_rgb = hsv_to_rgb(rgb_matrix_config.hsv);
        //rgb_matrix_set_color(28, led_rgb.r, led_rgb.g, led_rgb.b);
        rgb_matrix_set_color(CAPS_LOCK_LED_POS, indicator_color[0].r, indicator_color[0].g, indicator_color[0].b);
    }
    if (host_keyboard_led_state().num_lock) {
        rgb_matrix_set_color(NUM_LOCK_LED_POS, indicator_color[1].r, indicator_color[1].g, indicator_color[1].b);
    }
    //rgb灯效关闭时，手动更新
    if (rgb_matrix_config.enable == 0) {
        if (host_keyboard_led_state().caps_lock == 0) {
            rgb_matrix_set_color(CAPS_LOCK_LED_POS, 0, 0, 0); //关闭capslock
        }
        if (host_keyboard_led_state().num_lock == 0) {
            rgb_matrix_set_color(NUM_LOCK_LED_POS, 0, 0, 0); //关闭capslock
        }
        rgb_matrix_update_pwm_buffers();
    }
    return false;
}
#endif