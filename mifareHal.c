#include "mifareHal.h"
#include "mfrc522.h"
#include "cardData.h"

MFRC522Ptr_t mfrc = NULL;

void mifareHalInit()
{
    mfrc = MFRC522_Init();
    PCD_Init(mfrc, MFRC522_SPI);
}

bool mifareIsInProximity()
{
    bool res = PICC_IsNewCardPresent(mfrc);
    return res;
}

bool mifareReadData(uint8_t *readBuff)
{
    printf("Read Mifare Data\n\r");
    PICC_ReadCardSerial(mfrc);
    // Read Page
    uint8_t size = 18;
    uint8_t page = CARD_DATA_PAGE;
    StatusCode res = MIFARE_Read(mfrc, page, readBuff, &size);
    if (STATUS_OK == res) {
        for (int i = 0; i < 16; i += 4) {
            printf("Page %02u: %02X %02X %02X %02X\n", page, readBuff[i], readBuff[i + 1], 
                    readBuff[i + 2], readBuff[i+ 3]);
            page++;
        }
        PICC_HaltA(mfrc);
    }

    return STATUS_OK == res;
}

bool mifareWriteData(uint8_t *writeBuff)
{
    PICC_ReadCardSerial(mfrc);
    StatusCode res = MIFARE_Write(mfrc, CARD_DATA_PAGE, writeBuff, 16);
    PICC_HaltA(mfrc);
    return STATUS_OK == res;
}