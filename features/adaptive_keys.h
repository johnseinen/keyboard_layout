#pragma once
 
#include QMK_KEYBOARD_H
 
bool process_adaptive_key(uint16_t keycode, keyrecord_t *record);

// What a key is about to type once the adaptive rules are applied (used by
// Sentence Case). Returns `basic` itself when no swap applies.
uint16_t adaptive_output_for(uint16_t basic, uint8_t *mods);