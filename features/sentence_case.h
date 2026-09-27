#pragma once

#include QMK_KEYBOARD_H

// Auto-capitalizes the first letter after a sentence-ending ".", "?" or "!"
// followed by a space. Trimmed-down replacement for getreuer/sentence_case:
// same core state machine (see https://getreuer.info/posts/keyboards/sentence-case),
// minus the abbreviation exceptions ("vs.", "etc."), the primed-state LED
// callback, the on/off/toggle keycodes, and debug logging. Reads adaptive
// key output directly via adaptive_output_for(), so a swap like Y+U -> "."
// still capitalizes the next letter with no callback override needed.
//
// Call process_sentence_case() from process_record_user(), BEFORE
// process_adaptive_key() so a one-shot shift set here still applies to the
// very next key. Call sentence_case_task() from matrix_scan_user() for the
// idle timeout.

// Resets the state machine, e.g. after a layer switch, hotkey, or anything
// else backspace can't undo.
void sentence_case_clear(void);

// Timeout-based reset; call every scan cycle.
void sentence_case_task(void);

// Feeds one keypress through the state machine. Always returns true (never
// suppresses the key) -- call it, then still pass the key to
// process_adaptive_key() / the rest of process_record_user().
bool process_sentence_case(uint16_t keycode, keyrecord_t *record);