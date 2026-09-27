#include "select_word.h"

// See select_word.h for what this drops from getreuer/select_word. Windows
// and Linux hotkeys only:
// dir < 0: Backward word selection: Ctrl+Left, Ctrl+Right, Ctrl+Shift+Left.
// dir > 0: Forward word selection: Ctrl+Right, Ctrl+Left, Ctrl+Shift+Right.

// Default to a timeout of 5 seconds.
#ifndef SELECT_WORD_TIMEOUT
#define SELECT_WORD_TIMEOUT 5000
#endif  // SELECT_WORD_TIMEOUT

static int8_t  selection_dir           = 0;
static bool    reset_before_next_event = false;
static uint8_t registered_hotkey       = KC_NO;

// Idle timeout timer to reset Select Word after a period of inactivity.
#if SELECT_WORD_TIMEOUT > 0
static uint16_t idle_timer = 0;

static void restart_idle_timer(void) {
    idle_timer = (timer_read() + SELECT_WORD_TIMEOUT) | 1;
}

void housekeeping_task_select_word(void) {
    if (idle_timer && timer_expired(timer_read(), idle_timer)) {
        idle_timer    = 0;
        selection_dir = 0;
    }
}
#endif  // SELECT_WORD_TIMEOUT > 0

static void clear_all_mods(void) {
    clear_mods();
    clear_weak_mods();
    clear_oneshot_mods();
}

static uint8_t select_init(void) {
    const uint8_t saved_mods = get_mods();
    clear_all_mods();
    reset_before_next_event = false;
    return saved_mods;
}

static void select_word_in_dir(int8_t dir) {
    // To extend an existing selection: dir < 0: Ctrl+Shift+Left.
    //                                  dir > 0: Ctrl+Shift+Right.
    const uint8_t saved_mods = select_init();

    if (selection_dir && (selection_dir < 0) != (dir < 0)) {  // Reversal.
        send_keyboard_report();
        tap_code_delay((dir < 0) ? KC_RGHT : KC_LEFT, TAP_CODE_DELAY);
    }

    add_mods(MOD_BIT_LCTRL);

    if (selection_dir == 0) {  // Initial selection.
        send_keyboard_report();
        send_string_with_delay_P(
            (dir < 0) ? PSTR(SS_TAP(X_LEFT) SS_TAP(X_RGHT))
                      : PSTR(SS_TAP(X_RGHT) SS_TAP(X_LEFT)),
            TAP_CODE_DELAY);
    }

    register_mods(MOD_BIT_LSHIFT);
    registered_hotkey = (dir < 0) ? KC_LEFT : KC_RGHT;
    register_code(registered_hotkey);

    set_mods(saved_mods);
    selection_dir = dir;
}

static void select_line(int8_t dir) {
    // dir < 0: Backward line selection: End, Shift+Home.
    // dir > 0: Forward line selection: Home, Shift+End.
    // Or to extend an existing selection: dir < 0: Shift+Up.
    //                                     dir > 0: Shift+Down.
    const uint8_t saved_mods = select_init();

    if (selection_dir != dir) {
        send_keyboard_report();

        if (selection_dir && (selection_dir < 0) != (dir < 0)) {  // Reversal.
            tap_code_delay((dir < 0) ? KC_LEFT : KC_RGHT, TAP_CODE_DELAY);
            selection_dir = 0;
        }

        if (selection_dir == 0) {  // Move cursor to the start/end of the line.
            tap_code16_delay((dir < 0) ? KC_END : KC_HOME, TAP_CODE_DELAY);
        }

        // Select to the opposite end of the line.
        tap_code16_delay((dir < 0) ? S(KC_HOME) : S(KC_END), TAP_CODE_DELAY);
    } else {
        register_mods(MOD_BIT_LSHIFT);
        registered_hotkey = (dir < 0) ? KC_UP : KC_DOWN;
        register_code(registered_hotkey);
    }

    set_mods(saved_mods);
    selection_dir = dir;
}

void select_word_register(char action) {
    if (registered_hotkey) {
        select_word_unregister();
    }

    switch (action) {
        case 'W':
            select_word_in_dir(1);
            break;
        case 'B':
            select_word_in_dir(-1);
            break;
        case 'L':
            select_line(2);
            break;
        case 'U':
            select_line(-2);
            break;
    }

#if SELECT_WORD_TIMEOUT > 0
    idle_timer = 0;
#endif  // SELECT_WORD_TIMEOUT > 0
}

void select_word_unregister(void) {
    reset_before_next_event = false;
    unregister_code(registered_hotkey);

    const uint8_t saved_mods = get_mods();
    clear_all_mods();

    switch (registered_hotkey) {
        case KC_DOWN:
            // Selecting multiple lines downward: tap Shift+End on release
            // to ensure the selection extends to the end of the last line.
            send_keyboard_report();
            send_string_with_delay_P(PSTR(SS_LSFT(SS_TAP(X_END))), TAP_CODE_DELAY);
            break;

        case KC_UP:
            // Selecting multiple lines upward: tap Shift+Home on release to
            // ensure the selection extends to the start of the first line.
            send_keyboard_report();
            send_string_with_delay_P(PSTR(SS_LSFT(SS_TAP(X_HOME))), TAP_CODE_DELAY);
            break;
    }

    set_mods(saved_mods);
    registered_hotkey = KC_NO;
#if SELECT_WORD_TIMEOUT > 0
    restart_idle_timer();
#endif  // SELECT_WORD_TIMEOUT > 0
}

bool process_record_select_word(uint16_t keycode, keyrecord_t *record) {
    if (selection_dir) {
        if (reset_before_next_event) {
            selection_dir = 0;
        }

        // Ignore modifier, one-shot, and momentary/toggle layer keys, and
        // hold events on mod-tap/layer-tap keys (e.g. the Space/Nav key
        // this all lives behind), so reaching for these keys doesn't
        // itself reset an in-progress selection.
        switch (keycode) {
            case MODIFIER_KEYCODE_RANGE:
            case QK_MOMENTARY ... QK_MOMENTARY_MAX:
            case QK_TO ... QK_TO_MAX:
            case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
            case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
            case QK_ONE_SHOT_MOD ... QK_ONE_SHOT_MOD_MAX:
                return true;
            case QK_MOD_TAP ... QK_MOD_TAP_MAX:
            case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
                if (record->tap.count == 0) {
                    return true;
                }
                break;
        }

        reset_before_next_event = true;
    }

#if SELECT_WORD_TIMEOUT > 0
    if (idle_timer) {
        restart_idle_timer();
    }
#endif  // SELECT_WORD_TIMEOUT > 0

    switch (keycode) {
        case SELECT_WORD:
            // No Shift-selects-the-line check here: SELECT_LINE already
            // covers that directly, and checking Shift risked selecting a
            // whole line by accident whenever Shift happened to still be
            // down (e.g. right after typing a capitalized word).
            if (record->event.pressed) {
                select_word_register('W');
            } else {
                select_word_unregister();
            }
            break;

        case SELECT_WORD_BACK:
            if (record->event.pressed) {
                select_word_register('B');
            } else {
                select_word_unregister();
            }
            break;

        case SELECT_LINE:
            if (record->event.pressed) {
                select_word_register('L');
            } else {
                select_word_unregister();
            }
            break;

        case SELECT_LINE_UP:
            if (record->event.pressed) {
                select_word_register('U');
            } else {
                select_word_unregister();
            }
            break;
    }

    return true;
}