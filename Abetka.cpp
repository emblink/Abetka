#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/uart.h"
#include "mfrc522.h"
// #include "ws2812.pio.h"

int main()
{
    stdio_init_all();
    // Declare card UID's
    // uint8_t tag1[] = {0x93, 0xE3, 0x9A, 0x92};
    uint8_t tag1[] = {0xEC, 0xD8, 0xC2, 0x01 };

    MFRC522Ptr_t mfrc = MFRC522_Init();
    PCD_Init(mfrc, spi0);
    sleep_ms(100);

    while(1)
    {
        //Wait for new card
        printf("Waiting for card\n\r");
        while(!PICC_IsNewCardPresent(mfrc));
        //Select the card
        printf("Selecting card\n\r");
        PICC_ReadCardSerial(mfrc);

        //Show UID on serial monitor
        printf("PICC dump: \n\r");
        PICC_DumpToSerial(mfrc, &(mfrc->uid));

        //Authorization with uid
        printf("Uid is: ");
        for(int i = 0; i < 4; i++) {
            printf("%x ", mfrc->uid.uidByte[i]);
        } printf("\n\r");

        if(memcmp(mfrc->uid.uidByte, tag1, 4) == 0) {
            printf("Authentication Success\n\r");
        } else {
            printf("Authentication Failed\n\r");
        }
    }
}
