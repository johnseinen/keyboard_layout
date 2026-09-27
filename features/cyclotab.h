#pragma once

#include QMK_KEYBOARD_H

// Trimmed clone of getreuer/cyclotab, kept to only what this keymap uses:
// CYCLOTAB_KEYS (A(KC_TAB), C(KC_TAB) here) held while cycling, committed by
// pressing any other key -- in practice Enter, or releasing the Nav layer
// key these live on -- and cancelled with Esc, which taps a literal Escape
// (most switcher UIs read Escape-while-modifier-held as "revert to the
// original window") before releasing the mod.
//
// Dropped vs. the original module, since nothing in this keymap uses them:
//   - Idle timeout / housekeeping task. This keymap already set
//     CYCLOTAB_TIMEOUT 0, which disabled the original's timeout too, so
//     there's no auto-commit here at all -- only Enter, Esc, or leaving the
//     layer ends a cycle.
//   - Arrow-key / Shift passthrough for 2-D switcher navigation. Arrow keys
//     also commit here rather than moving the highlight.
//   - The Shift-held / S(key) reverse-direction path (unused: nothing in
//     this keymap binds a shifted trigger) and REPEAT_KEY_ENABLED support.
//
// Call process_cyclotab() as the very first thing in process_record_user(),
// the same way process_adaptive_key() is gated, so a swallowed Enter/Esc
// never reaches Sentence Case or the adaptive rules:
//     if (!process_cyclotab(keycode, record)) { return false; }

bool process_cyclotab(uint16_t keycode, keyrecord_t *record);