#include "keyScan.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "pinout.h"

typedef struct {
    const uint gpio;
    int32_t threshold;
    uint32_t holdUs;
    KeyState lastState;
    KeyState currentState;
    bool wasPressed;
} KeyData;

#define KEYSCAN_INTERVAL_US 500
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

            // should mark it as pressed but not report yet, maybe it's a hold action
            if (KEY_STATE_HOLD != keys[key].currentState) {
                keys[key].wasPressed = true;
                keys[key].holdUs += KEYSCAN_INTERVAL_US;
                if (keys[key].holdUs >= HOLD_DURATION_US) {
                    keys[key].currentState = KEY_STATE_HOLD;
                    keys[key].holdUs = 0;
                    keys[key].wasPressed = false;
                }
            }
        } else if (keys[key].threshold <= RELEASE_THRESHOLD) {
            keys[key].threshold = RELEASE_THRESHOLD;

            if (keys[key].wasPressed) { // report short press after key release if hold didn't happend
                keys[key].currentState = KEY_STATE_PRESSED;
                keys[key].wasPressed = false;
            }
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
            return true;
        default:
            break;
    }

    return false;
}

bool keyScanGetKeyState(Key key)
{
    assert(key < KEY_COUNT);

    int32_t threshold = 0;
    for (int i = 0; i < PRESS_THRESHOLD; i++) {
        threshold += !gpio_get(keys[key].gpio) ? 1 : -1;
    }

    if (threshold >= PRESS_THRESHOLD) {
        return true;
    }

    return false;
}

void keyScanProcess()
{
    struct {
        Key key;
        KeyState state;
    } pendingEvents[KEY_COUNT] = {0};
    int pendingEventCount = 0;

    uint32_t status = save_and_disable_interrupts();
    for (Key key = KEY_LEFT; key < KEY_COUNT; key++) {
        if (keys[key].currentState != keys[key].lastState) {
            // Store the event for later processing outside of critical section
            pendingEvents[pendingEventCount].key = key;
            pendingEvents[pendingEventCount].state = keys[key].currentState;
            pendingEventCount++;
            keys[key].lastState = keys[key].currentState;
            if (KEY_STATE_PRESSED == keys[key].currentState) {
                keys[key].currentState = KEY_STATE_RELEASED;
            }
        }
    }
    restore_interrupts(status);

    // Process pending events *outside* the critical section
    for (size_t i = 0; i < pendingEventCount; i++) {
        if (keyCb) {
            keyCb(pendingEvents[i].key, pendingEvents[i].state);
        }
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