#include QMK_KEYBOARD_H
#include "sentence_case.h"
#include "adaptive_keys.h"
#include <string.h>

// See sentence_case.h for what this trims from getreuer/sentence_case.

#ifndef SENTENCE_CASE_TIMEOUT
#define SENTENCE_CASE_TIMEOUT 5000  // ms; 0 disables the idle reset.
#endif

#define SC_HIST_SIZE 4  // How many keys back Backspace can rewind.

enum { SC_INIT, SC_WORD, SC_ENDING, SC_PRIMED };

static uint8_t  sc_state   = SC_INIT;
static uint8_t  sc_hist[SC_HIST_SIZE];
static uint16_t sc_suppress_key = KC_NO;
#if SENTENCE_CASE_TIMEOUT > 0
static uint16_t sc_idle_timer = 0;
#endif

// Classifies a key the same way getreuer/sentence_case's default
// sentence_case_press_user() does, minus non-US-letter customization.
static char sc_classify(uint16_t keycode, uint8_t mods) {
    if (mods & ~MOD_MASK_SHIFT) {
        return '\0';  // A non-shift modifier: ignore, don't touch state.
    }

    bool shifted = mods & MOD_MASK_SHIFT;

    if (keycode >= KC_A && keycode <= KC_Z) {
        return 'a';
    }

    switch (keycode) {
        case KC_DOT:
            return shifted ? '#' : '.';
        case KC_SLSH:  // '?' when shifted
            return shifted ? '.' : '#';
        case KC_1:  // '!' when shifted
            return shifted ? '.' : '#';
        case KC_QUES:  // Direct '?' key on the Sym layer -- ends a sentence
        case KC_EXLM:  // regardless of any shift state.
            return '.';
        case KC_SPC:
            return ' ';
        case KC_QUOT:
        case KC_DQUO:  // Quotes don't change state, so a sentence ending in
            return '\''; // a quote mark still primes capitalization.
        default:
            return '#';  // Any other symbol/digit: backspaceable, breaks a word.
    }
}

static void sc_push(uint8_t new_state) {
    for (uint8_t i = SC_HIST_SIZE - 1; i > 0; i--) {
        sc_hist[i] = sc_hist[i - 1];
    }
    sc_hist[0] = sc_state;
    sc_state   = new_state;
}

void sentence_case_clear(void) {
    sc_state = SC_INIT;
    memset(sc_hist, SC_INIT, sizeof(sc_hist));
    sc_suppress_key = KC_NO;
#if SENTENCE_CASE_TIMEOUT > 0
    sc_idle_timer = 0;
#endif
}

void sentence_case_task(void) {
#if SENTENCE_CASE_TIMEOUT > 0
    if (sc_idle_timer && timer_expired(timer_read(), sc_idle_timer)) {
        sentence_case_clear();
    }
#endif
}

bool process_sentence_case(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }

    if (keycode == KC_BSPC) {
        sc_state = sc_hist[SC_HIST_SIZE - 1];
        for (uint8_t i = SC_HIST_SIZE - 1; i > 0; i--) {
            sc_hist[i] = sc_hist[i - 1];
        }
        sc_hist[0] = SC_INIT;
        return true;
    }

    uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();
    // See what this key will really type once adaptive rules run, so a
    // swap like Y+U -> "." is classified as the "." it becomes.
    keycode = adaptive_output_for(keycode, &mods);

#if SENTENCE_CASE_TIMEOUT > 0
    sc_idle_timer = (record->event.time + SENTENCE_CASE_TIMEOUT) | 1;
#endif

    switch (sc_classify(keycode, mods)) {
        case 'a':
            if (sc_state == SC_PRIMED) {
                if (keycode != sc_suppress_key) {
                    sc_suppress_key = keycode;
                    set_oneshot_mods(MOD_BIT(KC_LSFT));
                    sc_push(SC_WORD);
                }
            } else {
                sc_push(SC_WORD);
            }
            break;

        case '.':
            sc_push(sc_state == SC_WORD ? SC_ENDING : SC_INIT);
            break;

        case ' ':
            sc_push(sc_state == SC_ENDING || sc_state == SC_PRIMED ? SC_PRIMED : SC_INIT);
            break;

        case '\'':
            sc_push(sc_state);  // Quotes don't change state.
            break;

        default:  // '#' or '\0'
            sc_push(SC_INIT);
            break;
    }

    return true;
}