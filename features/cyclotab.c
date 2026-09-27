#include QMK_KEYBOARD_H
#include "cyclotab.h"

// See cyclotab.h for what this drops from getreuer/cyclotab.

#ifndef CYCLOTAB_KEYS
#define CYCLOTAB_KEYS A(KC_TAB)
#endif

static const uint16_t cyclotab_keys[] = {CYCLOTAB_KEYS};
#define NUM_CYCLOTAB_KEYS (ARRAY_SIZE(cyclotab_keys))

static uint16_t active_key  = KC_NO;  // the CYCLOTAB_KEYS entry now held
static uint8_t  active_mods = 0;      // its actual mod bits, for unregistering

// Note: unlike the original, this doesn't check IS_QK_MODS(). That check
// only mattered for computing S(cyclotab_key), the shifted-reverse variant,
// which this trimmed version doesn't support -- so a plain equality check
// is enough here.
static bool is_trigger(uint16_t keycode) {
    for (uint8_t i = 0; i < NUM_CYCLOTAB_KEYS; i++) {
        if (keycode == cyclotab_keys[i]) {
            return true;
        }
    }
    return false;
}

bool process_cyclotab(uint16_t keycode, keyrecord_t *record) {
    if (active_key != KC_NO) {
        if (keycode == active_key) {
            // Same trigger pressed again: let QMK's own A()/C() handling
            // re-tap the base key. Our mods are already registered, so this
            // just cycles forward without touching them.
            return true;
        }

        if (keycode == KC_ESC && record->event.pressed) {
            // Tap a literal Escape while the mod is still held, so the
            // switcher UI reverts to the original window before we let go.
            // Gated to the press only -- calling this on Esc's release too
            // would send a second, needless Escape.
            tap_code(KC_ESC);
        }

        // Any other key ends the cycle: commit (or, for Esc, cancel).
        // This covers Enter, Esc, and releasing the Nav layer key these
        // live on alike.
        unregister_mods(active_mods);
        active_key = KC_NO;

        // Swallow presses (so e.g. Enter doesn't also get typed into the
        // now-focused window); let releases through unchanged.
        return !record->event.pressed;
    }

    if (record->event.pressed && is_trigger(keycode)) {
        active_key      = keycode;
        uint8_t raw_mods = mod_config(QK_MODS_GET_MODS(active_key));
        active_mods      = ((raw_mods & 0x10) == 0) ? raw_mods : (raw_mods << 4);
        register_mods(active_mods);
        // Fall through: let QMK also process this same keypress normally,
        // so the first Tab actually gets sent.
    }

    return true;
}