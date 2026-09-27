MCU = atmega32u4
BOOTLOADER = caterina
 
UNICODE_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
MOUSEKEY_ENABLE = yes
RGBLIGHT_ENABLE = no
 
SPLIT_KEYBOARD = yes
BLUETOOTH_ENABLE = no
LTO_ENABLE = yes
NKRO_ENABLE = yes
REPEAT_KEY_ENABLE = yes
COMBOS_ENABLE = no
SRC += features/adaptive_keys.c
SRC += features/sentence_case.c
SRC += features/cyclotab.c
SRC += features/select_word.c