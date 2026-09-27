#pragma once

#define TAPPING_TERM 170
#define QUICK_TAP_TERM 120
#define FLOW_TAP_TERM 140
#define USB_SUSPEND_WAKEUP_DELAY 0
#define EE_HANDS
#define ONESHOT_TIMEOUT 1500
#define ONESHOT_TAP_TOGGLE 2
#define SPLIT_USB_DETECT

#define ADAPTIVE_KEYS_ENABLE
#define ADAPTIVE_TERM 1000

#define CYCLOTAB_KEYS A(KC_TAB), C(KC_TAB)
#define CYCLOTAB_TIMEOUT 0

#define MK_3_SPEED
#define MK_MOMENTARY_ACCEL

// Cursor (Synced to 165 Hz monitor frame timing)
#define MK_C_OFFSET_UNMOD 7     // ≈1167 px/s  (Default — ~1.6s across 1920px)
#define MK_C_INTERVAL_UNMOD 6
#define MK_C_OFFSET_0 2         // ≈333 px/s   (ACL0 — Precision, up from ~166 px/s — ~5.8s across 1920px)
#define MK_C_INTERVAL_0 6       // Lowered from 12 to 6 to eliminate 165Hz micro-stutter
#define MK_C_OFFSET_2 12        // ≈2000 px/s  (ACL2 — Warp speed — ~0.96s across 1920px)
#define MK_C_INTERVAL_2 6

// Wheel
#define MK_W_OFFSET_UNMOD 1      // ≈11 steps/s
#define MK_W_INTERVAL_UNMOD 90 
#define MK_W_OFFSET_0 1          // ≈8 steps/s  (precision scroll)
#define MK_W_INTERVAL_0 120
#define MK_W_OFFSET_2 2          // ≈33 steps/s (fast)
#define MK_W_INTERVAL_2 60