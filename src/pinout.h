#pragma once

#define ST7789_SPI              spi1
#define SD_CARD_SPI             spi0
#define MFRC522_SPI             NULL

#define I2S_PIO                 pio1
#define I2S_PIO_SM              0

#define WS2812_PIO              pio0
#define WS2812_PIO_SM           0

#define SPI_PIO                 pio0
#define SPI_PIO_CMD_SM          1

#define I2S_PIO_BLK             8
#define I2S_PIO_LRC             9
#define I2S_PIO_DIN             22
#define I2S_PIO_BASE            PIO1_BASE

#define MAX_98357A_SHUTDOWN     6

#define WS2812_PIN              23

#define SPI_PIO_MOSI            13
#define SPI_PIO_MISO            12
#define SPI_PIO_CLK             14
#define SPI_PIO_CS              15

#define ST7789_DIN              11
#define ST7789_CLK              10
#define ST7789_CS               -1
#define ST7789_DC               5
#define ST7789_RST              4
#define ST7789_BL               -1

#define MFRC522_MOSI            SPI_PIO_MOSI
#define MFRC522_MISO            SPI_PIO_MISO
#define MFRC522_SCK             SPI_PIO_CLK
#define MFRC522_CS              SPI_PIO_CS
#define MFRC522_RST             17

#define SD_CARD_MOSI            19
#define SD_CARD_MISO            16
#define SD_CARD_SCK             18
#define SD_CARD_CS              20

#define BATTERY_ADC_PIN         26
#define BATTERY_ADC_CHANNEL     0

#define KEY_LEFT_GPIO           3
#define KEY_RIGHT_GPIO          7
#define KEY_UP_GPIO             1
#define KEY_DOWN_GPIO           2
#define KEY_SELECT_GPIO         0
#define KEY_USER_GPIO           24
