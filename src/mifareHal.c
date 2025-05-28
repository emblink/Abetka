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

bool mifareReadData(CardData *card)
{
    printf("Read Mifare Data\n\r");
    if (!PICC_ReadCardSerial(mfrc)) {
        printf("Failed to read card serial\n\r");
        return false;
    }

    // Read 2 Pages, because only 4 bytes per page can be stored, see MIFARE_Write description
    uint8_t readBuff[18] = {0};
    uint8_t size = 18;
    StatusCode res = STATUS_ERROR;

    res = MIFARE_Read(mfrc, CARD_LANGUAGE_PAGE, readBuff, &size);
    if (STATUS_OK != res) {
        printf("Failed to read card language page\n\r");
        return false;
    }
    printf("Card Language: "); 
    for (int i = 0; i < sizeof(card->langName); i++) {
        card->langName[i] = readBuff[i];
        printf("%c", readBuff[i]); 
    }
    printf("\n");

    res = MIFARE_Read(mfrc, CARD_SYMBOL_PAGE, readBuff, &size);
    if (STATUS_OK != res) {
        printf("Failed to read card symbol page\n\r");
        return false;
    }
    printf("Card Symbol: "); 
    for (int i = 0; i < sizeof(card->langName); i++) {
        card->symbolUtf8[i] = readBuff[i];
        printf("%c", readBuff[i]);
    }
    printf("\n");

    PICC_HaltA(mfrc);

    return true;
}

bool mifareWriteData(CardData *card)
{
    if (!PICC_ReadCardSerial(mfrc)) {
        printf("Failed to read card serial\n\r");
        return false;
    }

    uint8_t writeBuff[16] = {0};
    for (int i = 0; i < sizeof(card->langName); i++) {
        writeBuff[i] = card->langName[i];
    }

    StatusCode res = STATUS_ERROR;
    res = MIFARE_Write(mfrc, CARD_LANGUAGE_PAGE, writeBuff, 16);
    if (STATUS_OK != res) {
        printf("Failed to write card langugae page\n\r");
        return false;
    }

    for (int i = 0; i < sizeof(card->symbolUtf8); i++) {
        writeBuff[i] = card->symbolUtf8[i];
    }
    res = MIFARE_Write(mfrc, CARD_SYMBOL_PAGE, writeBuff, 16);
    if (STATUS_OK != res) {
        printf("Failed to write card symbol page\n\r");
        return false;
    }

    PICC_HaltA(mfrc);
    return true;
}