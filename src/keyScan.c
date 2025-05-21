#include "keyScan.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"

typedef struct {
    const uint gpio;
    int32_t threshold;
    uint32_t holdUs;
    KeyState lastState;
    KeyState currentState;
} KeyData;

#define KEYSCAN_INTERVAL_US 100
#define PRESS_THRESHOLD 20
#define RELEASE_THRESHOLD -20

static volatile KeyData keys[KEY_COUNT] = {
    [KEY_LEFT].gpio = KEY_LEFT_GPIO,
    [KEY_RIGHT].gpio = KEY_RIGHT_GPIO,
    [KEY_USER].gpio = KEY_USER_GPIO,
};
static volatile KeyCallback keyCb = NULL;
static volatile bool isKeyScanTimerActive = false;
static repeating_timer_t keyScanTimer;

static bool isAnyKeyPressed()
{
    bool isPressed = false;
    for (uint key = KEY_LEFT; key < KEY_COUNT; key++) {
        isPressed |= keyScanIsKeyPressed(key);
    }

    return isPressed;
}

bool keyScanTimercb(repeating_timer_t *t) {
    bool anyPressed = false;

    for (uint key = KEY_LEFT; key < KEY_COUNT; key++) {
        keys[key].threshold += !gpio_get(keys[key].gpio) ? 1 : -1;
        
        if (keys[key].threshold >= PRESS_THRESHOLD) {
            anyPressed = true;
            keys[key].threshold = PRESS_THRESHOLD;

            if (KEY_STATE_RELEASED == keys[key].currentState) {
                keys[key].currentState = KEY_STATE_PRESSED;
                keys[key].holdUs = 0;
            } else if (KEY_STATE_PRESSED == keys[key].currentState) {
                keys[key].holdUs += KEYSCAN_INTERVAL_US;
                if (keys[key].holdUs >= HOLD_DURATION_US) {
                    keys[key].currentState = KEY_STATE_HOLD;
                    keys[key].holdUs = HOLD_DURATION_US;
                }
            }
        } else if (keys[key].threshold <= RELEASE_THRESHOLD) {
            keys[key].threshold = RELEASE_THRESHOLD;
            keys[key].currentState = KEY_STATE_RELEASED;
            keys[key].holdUs = 0;
        } else {
            anyPressed = true; // still bouncing
        }
    }

    if (!anyPressed) {
        cancel_repeating_timer(&keyScanTimer);
        isKeyScanTimerActive = false;
        return false; // stop timer
    }

    return true;
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    if (!isKeyScanTimerActive) {
        isKeyScanTimerActive = true;
        add_repeating_timer_us(-KEYSCAN_INTERVAL_US, keyScanTimercb, NULL, &keyScanTimer);
    }
}

static void keyInit(Key key)
{
    keys[key].lastState = KEY_STATE_RELEASED;
    keys[key].currentState = KEY_STATE_RELEASED;
    keys[key].holdUs = 0;
    keys[key].threshold = 0;

    uint gpio = keys[key].gpio;
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
    gpio_set_irq_enabled_with_callback(gpio,  GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);
}

void keyScanInit(KeyCallback callback)
{
    keyScanDeinit();
    assert(callback);
    keyCb = callback;

    for (uint key = KEY_LEFT; key < KEY_COUNT; key++) {
        keyInit(key);
    }
}

bool keyScanIsKeyPressed(Key key)
{
    assert(key < KEY_COUNT);

    switch (keys[key].currentState) {
        case KEY_STATE_PRESSED:
        case KEY_STATE_HOLD:
        case KEY_STATE_LONG_HOLD:
            return true;
        default:
            break;
    }

    return false;
}

void keyScanProcess()
{
    for (Key key = KEY_LEFT; key < KEY_COUNT; key++) {
        uint32_t status = save_and_disable_interrupts();
        if (keys[key].currentState != keys[key].lastState) {
            keys[key].lastState = keys[key].currentState;
            if (keyCb) {
                keyCb(key, keys[key].currentState);
            }
        }
        restore_interrupts(status);
    }
}

void keyScanDeinit()
{
    keyCb = NULL;
    uint32_t status = save_and_disable_interrupts();
    cancel_repeating_timer(&keyScanTimer);
    isKeyScanTimerActive = false;
    restore_interrupts(status);
}