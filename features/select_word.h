#pragma once

#include QMK_KEYBOARD_H

// Trimmed clone of getreuer/select_word, Windows/Linux only -- see
// https://getreuer.info/posts/keyboards/select-word for the original.
//
// Selects the word under the cursor (Ctrl+Left/Right dance), or the current
// line with Shift held. HOLD the key (don't release) to extend the
// selection: this registers the underlying Ctrl+Shift+Arrow (or
// Shift+Home/End) as a genuinely held key, so the OS's own key-repeat
// extends it for as long as you hold it -- releasing and re-pressing starts
// a fresh selection at the new cursor position instead.
//
// Four action keycodes, defined below via SELECT_WORD_SAFE_RANGE so
// keymap.c's own custom keycodes can continue right after them without
// colliding:
//   SELECT_WORD       -- forward word select; Shift+press selects the line.
//   SELECT_WORD_BACK  -- backward word select.
//   SELECT_LINE       -- forward line select.
//   SELECT_LINE_UP    -- backward (upward) line select.
//
// Dropped vs. the original module, since nothing in this keymap uses them:
//   - macOS support and OS detection (SELECT_WORD_OS_MAC/_DYNAMIC,
//     OS_DETECTION_ENABLE). Hotkeys are hardcoded to the Windows/Linux set.
//   - The QK_LAYER_MOD and QK_LAYER_TAP_TOGGLE ignore-cases (no LM() or
//     TT() keys in this keymap) and the LAYER_LOCK_ENABLE case (module not
//     used).
//   - The NO_ACTION_ONESHOT / NO_ACTION_TAPPING guards around the one-shot
//     and tap-hold handling: this keymap uses both features, so those
//     branches are compiled in unconditionally rather than behind an
//     #ifndef. If NO_ACTION_ONESHOT or NO_ACTION_TAPPING is ever added to
//     config.h, this file needs the guards put back.
//
// Kept as-is, since it's the subtle part: the reset_before_next_event /
// idle-timeout state machine that decides when an in-progress selection
// should be treated as continuing vs. reset, unchanged from upstream.

enum {
    SELECT_WORD = SAFE_RANGE,
    SELECT_WORD_BACK,
    SELECT_LINE,
    SELECT_LINE_UP,
    SELECT_WORD_SAFE_RANGE,  // keymap.c's own custom keycodes start here.
};

void select_word_register(char action);
void select_word_unregister(void);
bool process_record_select_word(uint16_t keycode, keyrecord_t *record);
void housekeeping_task_select_word(void);