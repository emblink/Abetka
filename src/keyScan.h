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
} KeyState;

typedef enum {
    KEY_LEFT = 0U,
    KEY_RIGHT,
    KEY_USER,
    KEY_COUNT,
} Key;

typedef void (* KeyCallback)(Key key, KeyState event);
typedef void (* IdleCallback)(void);

#define HOLD_DURATION_US (1000 * 1000)

void keyScanInit(KeyCallback callback);
void keyScanSetIdleCallback(IdleCallback callback);
bool keyScanIsKeyPressed(Key key); // get buffered value
bool keyScanGetKeyState(Key key); // measure current state
void keyScanProcess();
void keyScanDeinit();

#ifdef __cplusplus
}
#endif