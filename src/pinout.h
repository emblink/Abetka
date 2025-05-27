#pragma once

#define ST7789_SPI              spi1
#define SD_CARD_SPI             spi0
#define MFRC522_SPI             spi0

#define I2S_PIO                 pio1
#define I2S_PIO_SM              0

#define WS2812_PIO              pio0
#define WS2812_PIO_SM           0

#define SPI_PIO                 pio0
#define SPI_PIO_CMD_SM          1

#define I2S_PIO_BLK             6
#define I2S_PIO_DIN             8
#define I2S_PIO_BASE            PIO1_BASE

#define WS2812_PIN              23

#define SPI_PIO_MOSI            13
#define SPI_PIO_MISO            4
#define SPI_PIO_CLK             5
#define SPI_PIO_CS              17

#define ST7789_DIN              15
#define ST7789_CLK              14
#define ST7789_CS               -1
#define ST7789_DC               11
#define ST7789_RST              10
#define ST7789_BL               5 // Backlight

#define MFRC522_MOSI            SPI_PIO_MOSI
#define MFRC522_MISO            SPI_PIO_MISO
#define MFRC522_SCK             SPI_PIO_CLK
#define MFRC522_CS              SPI_PIO_CS
#define MFRC522_RST             12

#define SD_CARD_MOSI            19
#define SD_CARD_MISO            16
#define SD_CARD_SCK             18
#define SD_CARD_CS              20

#define BATTERY_ADC_PIN         29
#define BATTERY_ADC_CHANNEL     3

#define KEY_LEFT_GPIO           21
#define KEY_RIGHT_GPIO          22
#define KEY_UP_GPIO             26
#define KEY_DOWN_GPIO           27
#define KEY_SELECT_GPIO         28
#define KEY_USER_GPIO           24
