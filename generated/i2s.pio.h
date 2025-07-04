// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ---------------- //
// i2s_pio_dma_mono //
// ---------------- //

#define i2s_pio_dma_mono_wrap_target 0
#define i2s_pio_dma_mono_wrap 10
#define i2s_pio_dma_mono_pio_version 0

#define i2s_pio_dma_mono_offset_entry_point_m 8u

static const uint16_t i2s_pio_dma_mono_program_instructions[] = {
            //     .wrap_target
    0x6101, //  0: out    pins, 1         side 0 [1]
    0x0940, //  1: jmp    x--, 0          side 1 [1]
    0x7001, //  2: out    pins, 1         side 2
    0xb0e6, //  3: mov    osr, isr        side 2
    0xb922, //  4: mov    x, y            side 3 [1]
    0x7101, //  5: out    pins, 1         side 2 [1]
    0x1945, //  6: jmp    x--, 5          side 3 [1]
    0x6001, //  7: out    pins, 1         side 0
    0x80a0, //  8: pull   block           side 0
    0xa8c7, //  9: mov    isr, osr        side 1
    0xa822, // 10: mov    x, y            side 1
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program i2s_pio_dma_mono_program = {
    .instructions = i2s_pio_dma_mono_program_instructions,
    .length = 11,
    .origin = -1,
    .pio_version = i2s_pio_dma_mono_pio_version,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0
#endif
};

static inline pio_sm_config i2s_pio_dma_mono_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + i2s_pio_dma_mono_wrap_target, offset + i2s_pio_dma_mono_wrap);
    sm_config_set_sideset(&c, 2, false, false);
    return c;
}
#endif

// ------------------ //
// i2s_pio_dma_stereo //
// ------------------ //

#define i2s_pio_dma_stereo_wrap_target 0
#define i2s_pio_dma_stereo_wrap 7
#define i2s_pio_dma_stereo_pio_version 0

#define i2s_pio_dma_stereo_offset_entry_point_s 7u

static const uint16_t i2s_pio_dma_stereo_program_instructions[] = {
            //     .wrap_target
    0x6001, //  0: out    pins, 1         side 0
    0x0840, //  1: jmp    x--, 0          side 1
    0x7001, //  2: out    pins, 1         side 2
    0xb822, //  3: mov    x, y            side 3
    0x7001, //  4: out    pins, 1         side 2
    0x1844, //  5: jmp    x--, 4          side 3
    0x6001, //  6: out    pins, 1         side 0
    0xa822, //  7: mov    x, y            side 1
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program i2s_pio_dma_stereo_program = {
    .instructions = i2s_pio_dma_stereo_program_instructions,
    .length = 8,
    .origin = -1,
    .pio_version = i2s_pio_dma_stereo_pio_version,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0
#endif
};

static inline pio_sm_config i2s_pio_dma_stereo_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + i2s_pio_dma_stereo_wrap_target, offset + i2s_pio_dma_stereo_wrap);
    sm_config_set_sideset(&c, 2, false, false);
    return c;
}
#endif

