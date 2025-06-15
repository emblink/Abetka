/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _PICO_ST7789_H_
#define _PICO_ST7789_H_

#include "hardware/spi.h"

struct st7789_config {
    spi_inst_t* spi;
    uint gpio_din;
    uint gpio_clk;
    int gpio_cs;
    uint gpio_dc;
    uint gpio_rst;
    int gpio_bl;
};

void st7789_init(const struct st7789_config* config, uint16_t width, uint16_t height);
void st7789_cmd(uint8_t cmd, const uint8_t* data, size_t len);
void st7789_write(const void* data, size_t len);
void st7789_put(uint16_t pixel);
void st7789_fill(uint16_t pixel);
void st7789_set_cursor(uint16_t x, uint16_t y);
void st7789_vertical_scroll(uint16_t row);
void st7789_ramwr();
void st7789_caset(uint16_t xs, uint16_t xe);
void st7789_raset(uint16_t ys, uint16_t ye);
void st7789_set_rotation(uint8_t madctl); // 0° - 0x00, 90° - 0x60, 180° - 0xC0, 270° - 0xA0

#endif

#ifdef __cplusplus
}
#endif