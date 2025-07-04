#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    KEY_STATE_RELEASED = 0U,
    KEY_STATE_PRESSED,
    KEY_STATE_HOLD,
    KEY_STATE_LONG_HOLD,
} KeyState;

typedef enum {
    KEY_LEFT = 0U,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_SELECT,
    KEY_USER,
    KEY_COUNT,
} Key;

typedef void (* KeyCallback)(Key key, KeyState event);

#define HOLD_DURATION_US (1000 * 1000)

void keyScanInit(KeyCallback callback);
bool keyScanIsKeyPressed(Key key);
void keyScanProcess();
void keyScanDeinit();

#ifdef __cplusplus
}
#endif