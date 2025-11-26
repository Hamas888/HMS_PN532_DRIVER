#ifndef HMS_PN532_MIFAREULTRALIGHT_H
#define HMS_PN532_MIFAREULTRALIGHT_H

#include "HMS_PN532_Config.h"
#include "HMS_PN532_NFC_Tag.h"
#include "HMS_PN532_Controller.h"

#define MIFAREULTRALIGHT_TYPE_NAME                  "NFC Forum Type 2"

#define MIFAREULTRALIGHT_PAGE_SIZE                  4
#define MIFAREULTRALIGHT_READ_SIZE                  4

#define MIFAREULTRALIGHT_DATA_START_PAGE            4
#define MIFAREULTRALIGHT_MESSAGE_LENGTH_INDEX       1
#define MIFAREULTRALIGHT_DATA_START_INDEX           2
#define MIFAREULTRALIGHT_MAX_PAGE                   63

class HMS_PN532_MifareUltralight {
    public:
        HMS_PN532_MifareUltralight(HMS_PN532_Controller& controller);
        ~HMS_PN532_MifareUltralight();

        HMS_PN532_StatusTypeDef cleanTag();
        HMS_PN532_NFC_Tag readTag(byte *uid, uint8_t uidLength);
        HMS_PN532_StatusTypeDef writeTag(HMS_PN532_NDEF_Message& ndefMessage, byte *uid, uint8_t uidLength);

    private:
        unsigned int            tagCapacity;
        unsigned int            messageLength;
        unsigned int            bufferSize;
        unsigned int            ndefStartIndex;
        HMS_PN532_Controller    *controller;

        bool isUnformatted();
        void findNdefMessage();
        void calculateBufferSize();
        void readCapabilityContainer();
};

#endif // HMS_PN532_MIFAREULTRALIGHT_H