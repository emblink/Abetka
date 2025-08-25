#include "pico/stdlib.h"

// Millisecond timestamp — you need this for `lv_tick_inc`
uint32_t getTimeMs(void) {
    return to_ms_since_boot(get_absolute_time());
}