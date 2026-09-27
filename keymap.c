#include QMK_KEYBOARD_H
#if __has_include("keymap.h")
#    include "keymap.h"
#endif

#include "features/select_word.h"
#include "features/adaptive_keys.h"
#include "features/sentence_case.h"
#include "features/cyclotab.h"

#define OSM_SHIFT_TIMEOUT 750
#define NUM_TAPPING_TERM 200

enum custom_keycodes {
    BASE = SELECT_WORD_SAFE_RANGE,
    NUM_OSM,
};

enum layers {
    _BASE,
    _QWERTY,
    _SYM,
    _NUM,
    _NAV,
    _FUNC,
    _MOUSE,
    _GAMING,
};

static uint16_t osm_shift_timer     = 0;
static uint16_t num_osm_timer       = 0;
static bool     num_osm_interrupted = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_cyclotab(keycode, record)) {
        return false;
    }

    process_record_select_word(keycode, record);
    process_sentence_case(keycode, record);

    if (!process_adaptive_key(keycode, record)) {
        return false;
    }

    if (record->event.pressed && keycode != NUM_OSM) {
        num_osm_interrupted = true;
    }

    switch (keycode) {
        case NUM_OSM:
            if (record->event.pressed) {
                num_osm_timer       = timer_read();
                num_osm_interrupted = false;
                layer_on(_NUM);
            } else {
                layer_off(_NUM);
                if (!num_osm_interrupted && timer_elapsed(num_osm_timer) < NUM_TAPPING_TERM) {
                    add_oneshot_mods(MOD_LSFT);
                }
            }
            return false;

        case BASE:
            if (record->event.pressed) {
                clear_oneshot_mods();
                reset_oneshot_layer();
                layer_move(_BASE);
            }
            return false;
    }
    return true;
}

void oneshot_mods_changed_user(uint8_t mods) {
    if (mods & MOD_MASK_SHIFT) {
        osm_shift_timer = timer_read();
    } else {
        osm_shift_timer = 0;
    }
}

void matrix_scan_user(void) {
    if (osm_shift_timer && timer_elapsed(osm_shift_timer) > OSM_SHIFT_TIMEOUT) {
        clear_oneshot_mods();
        osm_shift_timer = 0;
    }

    sentence_case_task();
    housekeeping_task_select_word();
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_W,        KC_C,           KC_M,       KC_P,        KC_K,          KC_J,       KC_Y,    KC_O,    KC_U,     KC_COMM,
        KC_R,        KC_S,           KC_T,       KC_H,        KC_F,          KC_QUOT,    KC_N,    KC_A,    KC_E,     KC_I,
        KC_Q,        KC_G,           KC_V,       KC_D,        KC_B,          KC_X,       KC_L,    KC_DOT,  KC_SCLN,  KC_Z,
                                                    LT(_NAV, KC_SPC), NUM_OSM,     MO(_SYM),  KC_BSPC
    ),

    [_SYM] = LAYOUT(
        KC_LBRC,     KC_RBRC,        KC_LCBR,    KC_RCBR,     KC_CIRC,       KC_PIPE,    KC_DLR,  KC_PLUS, KC_HASH,  KC_GRV,
        KC_QUES,     KC_ASTR,        KC_LPRN,    KC_RPRN,     KC_AMPR,       KC_DQUO,    KC_EQL,  KC_COLN, KC_UNDS,  KC_EXLM,
        KC_BSLS,     KC_MINS,        KC_LABK,    KC_RABK,     XXXXXXX,       XXXXXXX,    KC_PERC, KC_AT,   KC_SLSH,  KC_TILD,
                                                    QK_REP,           XXXXXXX,     XXXXXXX,  XXXXXXX
    ),

    [_NUM] = LAYOUT(
        KC_PERC,     KC_N,           S(KC_N),    KC_CIRC,     KC_DLR,        KC_H,       KC_J,    KC_K,    KC_L,     XXXXXXX,
        KC_6,        KC_3,           KC_1,       KC_2,        KC_ESC,        KC_SLSH,    KC_4,    KC_0,    KC_5,     KC_8,
        XXXXXXX,     KC_B,           KC_W,       KC_7,        XXXXXXX,       XXXXXXX,    KC_9,    S(KC_G), KC_DOT,   XXXXXXX,
                                                    LT(_NAV, KC_SPC), XXXXXXX,     XXXXXXX,  XXXXXXX
    ),

    [_NAV] = LAYOUT(
        OS_LGUI,          OS_LALT,        OS_LCTL,    C(KC_A),     C(KC_F),       OSL(_FUNC), SELECT_WORD_BACK, KC_UP,   SELECT_WORD, SELECT_LINE_UP,
        C(KC_Z),          KC_ESC,         KC_TAB,     A(KC_TAB),   C(KC_V),       BASE,       KC_LEFT,          KC_DOWN, KC_RIGHT,    SELECT_LINE,
        C(KC_S),          C(KC_C),        C(KC_X),    C(KC_TAB),   XXXXXXX,       XXXXXXX,    XXXXXXX,          XXXXXXX, XXXXXXX,  XXXXXXX,
                                                    XXXXXXX,          XXXXXXX,     KC_ENT,   TG(_MOUSE)
    ),


    [_MOUSE] = LAYOUT(
        XXXXXXX,     MS_BTN5,        MS_ACL2,    MS_WHLU,     XXXXXXX,       XXXXXXX,    XXXXXXX, MS_UP,   XXXXXXX,  XXXXXXX,
        XXXXXXX,     MS_BTN4,        MS_ACL0,    MS_WHLD,     XXXXXXX,       XXXXXXX,    MS_LEFT, MS_DOWN, MS_RGHT,  XXXXXXX,
        XXXXXXX,     XXXXXXX,        XXXXXXX,    XXXXXXX,     XXXXXXX,       XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX,
                                                    MS_BTN1,          MS_BTN2,     XXXXXXX,  TG(_MOUSE)
    ),

    [_FUNC] = LAYOUT(
        KC_F1,       KC_F2,          KC_F3,      KC_F4,       KC_F5,         KC_F6,      KC_F7,   KC_F8,   KC_F9,    KC_F10,
        XXXXXXX,     XXXXXXX,        TG(_GAMING),TG(_QWERTY), XXXXXXX,       KC_F11,     KC_F12,  XXXXXXX, XXXXXXX,  XXXXXXX,
        XXXXXXX,     XXXXXXX,        XXXXXXX,    XXXXXXX,     XXXXXXX,       XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX,
                                                    XXXXXXX,          XXXXXXX,     XXXXXXX,  XXXXXXX
    ),

    [_QWERTY] = LAYOUT(
        KC_Q,        KC_W,           KC_E,       KC_R,        KC_T,          KC_Y,       KC_U,    KC_I,    KC_O,     KC_P,
        KC_A,        KC_S,           KC_D,       KC_F,        KC_G,          KC_H,       KC_J,    KC_K,    KC_L,     KC_SCLN,
        KC_Z,        KC_X,           KC_C,       KC_V,        KC_B,          KC_N,       KC_M,    KC_COMM, KC_DOT,   KC_SLSH,
                                                    KC_SPC,           MO(_SYM),    KC_LSFT,  MO(_NAV)
    ),

    [_GAMING] = LAYOUT(
        KC_Q,        KC_W,           KC_E,       KC_R,        KC_T,          MS_WHLU,    XXXXXXX, MS_UP,   XXXXXXX,  MS_ACL2,
        KC_A,        KC_S,           KC_D,       KC_F,        KC_G,          MS_WHLD,    MS_LEFT, MS_DOWN, MS_RGHT,  MS_ACL0,
        KC_Z,        KC_X,           KC_C,       KC_V,        KC_B,          BASE,       XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX,
                                                    MS_BTN1,          MS_BTN2,     KC_LSFT,  KC_SPC
    ),
};
// clang-format on