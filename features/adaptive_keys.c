#include QMK_KEYBOARD_H
#include "adaptive_keys.h"
#include <progmem.h>
#include <stddef.h>

#ifdef ADAPTIVE_KEYS_ENABLE

// One rule = "after <prefix>, pressing <input> sends <output> instead".
// Only the single previous key is remembered, so a rule only ever sees one
// key of context.
typedef struct {
    uint16_t prefix;
    uint16_t input;
    uint16_t output;
} adaptive_rule_t;

#define SWAP(prefix, input, output) {prefix, input, output},

// PROGMEM keeps the table in flash instead of SRAM. Read out with memcpy_P.
static const adaptive_rule_t adaptive_rules[] PROGMEM = {
#include "adaptive_keys.def"
};

#undef SWAP

static uint16_t adaptive_prefix = KC_NO;   // last text key typed, KC_NO = none
static uint32_t adaptive_prefix_timer;
static uint16_t adaptive_suppressed_key = KC_NO;

static void adaptive_clear_prefix(void) {
    adaptive_prefix = KC_NO;
}

static void adaptive_set_prefix(uint16_t keycode) {
    adaptive_prefix       = keycode;
    adaptive_prefix_timer = timer_read32();
}

static bool adaptive_text_key(uint16_t keycode) {
    if ((keycode >= KC_A && keycode <= KC_Z) ||
        (keycode >= KC_1 && keycode <= KC_0)) {
        return true;
    }

    switch (keycode) {
        case KC_SPC:
        case KC_COMM:
        case KC_DOT:
        case KC_SCLN:
        case KC_QUOT:
        case KC_MINS:
        case KC_EQL:
        case KC_SLSH:
        case KC_BSLS:
        case KC_LBRC:
        case KC_RBRC:
        case KC_GRV:
            return true;
        default:
            return false;
    }
}

static uint16_t adaptive_tap_keycode(uint16_t keycode, keyrecord_t *record) {
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode)) {
        return record->tap.count ? get_tap_keycode(keycode) : KC_NO;
    }

    return keycode;
}

// Would pressing `basic` right now fire a swap? If so, fills *rule. Callers
// on the base layer already know that (process_adaptive_key returns early
// otherwise), so the layer check lives in adaptive_output_for below instead
// of here -- no point re-checking it on every firing key.
static bool adaptive_find_rule(uint16_t basic, uint8_t mods, adaptive_rule_t *rule) {
    if (adaptive_prefix == KC_NO ||
        timer_elapsed32(adaptive_prefix_timer) > ADAPTIVE_TERM ||
        (mods & ~MOD_MASK_SHIFT)) {
        return false;
    }

    for (uint8_t i = 0; i < ARRAY_SIZE(adaptive_rules); i++) {
        memcpy_P(rule, &adaptive_rules[i], sizeof(adaptive_rule_t));

        if (rule->prefix == adaptive_prefix && rule->input == basic) {
            return true;
        }
    }

    return false;
}

// Lets other code (Sentence Case) ask what a key is about to type. Returns the
// key that will really be sent, or `basic` itself if no swap applies. For
// non-letter outputs shift gets stripped when sending, so *mods is updated to
// match. Unlike process_adaptive_key, nothing has checked the layer yet, so
// this does it itself before the (more expensive) rule scan.
uint16_t adaptive_output_for(uint16_t basic, uint8_t *mods) {
    adaptive_rule_t rule;

    if (get_highest_layer(layer_state | default_layer_state) != 0 ||
        !adaptive_find_rule(basic, *mods, &rule)) {
        return basic;
    }

    if (!(rule.output >= KC_A && rule.output <= KC_Z)) {
        *mods &= ~MOD_MASK_SHIFT;
    }

    return rule.output;
}

bool process_adaptive_key(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        if (adaptive_suppressed_key == keycode) {
            adaptive_suppressed_key = KC_NO;
            return false;
        }
        return true;
    }

    // Rules match keycodes, not positions, so only run them on the base layer.
    // Otherwise they'd also fire on _QWERTY, _GAMING and the letters on _NUM.
    if (get_highest_layer(layer_state | default_layer_state) != 0) {
        adaptive_clear_prefix();
        return true;
    }

    uint16_t basic = adaptive_tap_keycode(keycode, record);

    // A tap-hold key that is being held (not tapped): leave the prefix alone.
    if (basic == KC_NO) {
        return true;
    }

    // Anything that isn't a plain keycode (shifted symbols like KC_LPRN,
    // layer keys, custom keycodes, one-shot mods...) breaks the sequence.
    if (keycode > QK_BASIC_MAX && !IS_QK_MOD_TAP(keycode) && !IS_QK_LAYER_TAP(keycode)) {
        adaptive_clear_prefix();
        return true;
    }

    // Plain modifier presses don't break the sequence.
    if (basic >= KC_LCTL && basic <= KC_RGUI) {
        return true;
    }

    uint8_t mods = get_mods() | get_oneshot_mods() | get_weak_mods();

#ifdef CAPS_WORD_ENABLE
    bool caps_word = is_caps_word_on();
    if (caps_word) {
        mods |= MOD_BIT(KC_LSFT);
    }
#endif

    // With only one key of context we can't "go back" to the key before the
    // one being deleted, so backspace just forgets the prefix. Side effect:
    // after a swap fires, Backspace + pressing the same key again types the
    // literal key instead of swapping again.
    if (basic == KC_BSPC) {
        adaptive_clear_prefix();
        return true;
    }

    adaptive_rule_t rule;
    if (adaptive_find_rule(basic, mods, &rule)) {
#ifdef CAPS_WORD_ENABLE
        if (caps_word) {
            process_caps_word(basic, record);
        }
#endif

        adaptive_suppressed_key = keycode;

        // Only letter outputs keep shift. For anything else (' and .)
        // shift is stripped first, so N + H under shift, one-shot shift
        // or caps word still gives ' and never ".
        bool    strip      = !(rule.output >= KC_A && rule.output <= KC_Z);
        uint8_t shift_mods = get_mods() & MOD_MASK_SHIFT;

        if (strip) {
            if (shift_mods) {
                unregister_mods(shift_mods);
            }
            del_weak_mods(MOD_MASK_SHIFT);
            del_oneshot_mods(MOD_MASK_SHIFT);
        }

        tap_code16(rule.output);

        if (strip && shift_mods) {
            register_mods(shift_mods);   // physical shift is still held
        }

        adaptive_set_prefix(rule.output);
        return false;
    }

    if (adaptive_text_key(basic) &&
        (!mods ||
         (basic >= KC_A && basic <= KC_Z &&
          !(mods & ~MOD_MASK_SHIFT)))) {
        adaptive_set_prefix(basic);
    } else {
        adaptive_clear_prefix();
    }

    return true;
}

#else  // !ADAPTIVE_KEYS_ENABLE

// Keep process_adaptive_key() defined even when the feature is disabled, so
// process_record_user() in keymap.c can call it unconditionally without its
// own #ifdef guard. With the feature off, every key just passes through.
bool process_adaptive_key(uint16_t keycode, keyrecord_t *record) {
    return true;
}

uint16_t adaptive_output_for(uint16_t basic, uint8_t *mods) {
    return basic;
}

#endif  // ADAPTIVE_KEYS_ENABLE