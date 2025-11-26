#include "HMS_PN532_MifareUltralight.h"

HMS_PN532_MifareUltralight::HMS_PN532_MifareUltralight(HMS_PN532_Controller& controller) {
    this->controller    = &controller;
    ndefStartIndex      = 0;
    messageLength       = 0;
}

HMS_PN532_MifareUltralight::~HMS_PN532_MifareUltralight() {

}

bool HMS_PN532_MifareUltralight::isUnformatted() {
    uint8_t page = 4;
    byte data[MIFAREULTRALIGHT_READ_SIZE];
    if (controller->mifareultralightReadPage(page, data) == HMS_PN532_OK) {
        return (data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF);
    } else {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Error. Failed read page %d", page);
        #endif
        return false;
    }
}

void HMS_PN532_MifareUltralight::findNdefMessage() {
    int page;
    byte data[12];                                      // 3 pages
    byte* data_ptr = &data[0];

    // the nxp read command reads 4 pages, unfortunately adafruit give me one page at a time
    HMS_PN532_StatusTypeDef status = HMS_PN532_OK;
    for (page = 4; page < 6; page++) {
        if(controller->mifareultralightReadPage(page, data_ptr) != HMS_PN532_OK) {
            status = HMS_PN532_ERROR;
        }
        #if HMS_PN532_DEBUG_ENABLED
            // pn532Logger.debug("Page %d - ", page);
            // for (int i = 0; i < MIFAREULTRALIGHT_PAGE_SIZE; i++) {
            //     ptr += sprintf(ptr, "%02X ", data_ptr[i]);
            // }
            // pn532Logger.debug("Data: %s", hexString);

        #endif
        data_ptr += MIFAREULTRALIGHT_PAGE_SIZE;
    }

    if (status == HMS_PN532_OK) {
        if (data[0] == 0x03) {
            messageLength = data[1];
            ndefStartIndex = 2;
        } else if (data[5] == 0x3) { // page 5 byte 1
            messageLength = data[6];
            ndefStartIndex = 7;
        }
    }

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("messageLength %d", messageLength);
        pn532Logger.debug("ndefStartIndex %d", ndefStartIndex);
    #endif
}

void HMS_PN532_MifareUltralight::calculateBufferSize() {
    bufferSize = messageLength + ndefStartIndex + 1;                // TLV terminator 0xFE is 1 byte

    if (bufferSize % MIFAREULTRALIGHT_READ_SIZE != 0) {
        bufferSize = (
            (bufferSize / MIFAREULTRALIGHT_READ_SIZE) + 1
        ) * MIFAREULTRALIGHT_READ_SIZE;                             // buffer must be an increment of page size
    }
}

void HMS_PN532_MifareUltralight::readCapabilityContainer() {
    byte data[MIFAREULTRALIGHT_PAGE_SIZE];

    if (controller->mifareultralightReadPage (3, data) == HMS_PN532_OK) {
        tagCapacity = data[2] * 8;                                                  // See AN1303 - different rules for Mifare Family byte2 = (additional data + 48)/8
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.debug("Tag capacity %d bytes", tagCapacity);
        #endif
    }
}

HMS_PN532_StatusTypeDef HMS_PN532_MifareUltralight::cleanTag() {
    uint8_t data[4] = { 0x00, 0x00, 0x00, 0x00 };                                       // factory tags have 0xFF, but OTP-CC blocks have already been set so we use 0x00
    
    readCapabilityContainer();                                                          // meta info for tag

    uint8_t pages = (
        tagCapacity / MIFAREULTRALIGHT_PAGE_SIZE
    ) + MIFAREULTRALIGHT_DATA_START_PAGE;

    for (int i = MIFAREULTRALIGHT_DATA_START_PAGE; i < pages; i++) {
        #if HMS_PN532_DEBUG_ENABLED
            // pn532Logger.debug("Writing page %d - ", i);
            // for (int i = 0; i < MIFAREULTRALIGHT_PAGE_SIZE; i++) {
            //     ptr += sprintf(ptr, "%02X ", data[i]);
            // }
            // pn532Logger.debug("Data: %s", hexString);
        #endif
        if (controller->mifareultralightWritePage(i, data) != HMS_PN532_OK) {
            return HMS_PN532_ERROR;
        }
    }
    return HMS_PN532_OK;
}

HMS_PN532_NFC_Tag HMS_PN532_MifareUltralight::readTag(byte * uid, uint8_t uidLength) {
    if (isUnformatted()) {
        Serial.println(F("WARNING: Tag is not formatted."));
        return HMS_PN532_NFC_Tag(uid, uidLength, MIFAREULTRALIGHT_TYPE_NAME);
    }

    readCapabilityContainer();                                                                      // meta info for tag
    findNdefMessage();
    calculateBufferSize();

    if (messageLength == 0) {                                                                       // data is 0x44 0x03 0x00 0xFE
        HMS_PN532_NDEF_Message message = HMS_PN532_NDEF_Message();
        message.addEmptyRecord();
        return HMS_PN532_NFC_Tag(uid, uidLength, MIFAREULTRALIGHT_TYPE_NAME, message);
    }

    uint8_t page;
    uint8_t index = 0;
    byte buffer[bufferSize];
    for (page = MIFAREULTRALIGHT_DATA_START_PAGE; page < MIFAREULTRALIGHT_MAX_PAGE; page++) {
        
        if (controller->mifareultralightReadPage(page, &buffer[index]) == HMS_PN532_OK) {          // read the data
            #ifdef MIFARE_ULTRALIGHT_DEBUG
            Serial.print(F("Page "));Serial.print(page);Serial.print(" ");
            nfc->PrintHexChar(&buffer[index], ULTRALIGHT_PAGE_SIZE);
            #endif
        } else {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Error. Failed read page %d", page);
            #endif
            messageLength = 0;
            break;
        }

        if (index >= (messageLength + ndefStartIndex)) {
            break;
        }

        index += MIFAREULTRALIGHT_DATA_START_PAGE;
    }

    HMS_PN532_NDEF_Message ndefMessage = HMS_PN532_NDEF_Message(&buffer[ndefStartIndex], messageLength);
    return HMS_PN532_NFC_Tag(uid, uidLength, MIFAREULTRALIGHT_TYPE_NAME, ndefMessage);

}

HMS_PN532_StatusTypeDef HMS_PN532_MifareUltralight::writeTag(HMS_PN532_NDEF_Message& ndefMessage, byte *uid, uint8_t uidLength) {
    if (isUnformatted()) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Tag is not formatted.");
        #endif
        return HMS_PN532_ERROR;
    }
    readCapabilityContainer();                                                                                                         // meta info for tag

    messageLength  = ndefMessage.getEncodedSize();
    ndefStartIndex = messageLength < 0xFF ? 2 : 4;
    calculateBufferSize();

    if(bufferSize>tagCapacity) {
	    #if HMS_PN532_DEBUG_ENABLED
    	pn532Logger.error("Encoded Message length exceeded tag Capacity ");
    	#endif
    	return HMS_PN532_ERROR;
    }

    uint8_t encoded[bufferSize];
    uint8_t *  src = encoded;
    unsigned int position = 0;
    uint8_t page = MIFAREULTRALIGHT_DATA_START_PAGE;

    // Set message size. With ultralight should always be less than 0xFF but who knows?

    encoded[0] = 0x3;
    if (messageLength < 0xFF) {
        encoded[1] = messageLength;
    } else {
        encoded[1] = 0xFF;
        encoded[2] = ((messageLength >> 8) & 0xFF);
        encoded[3] = (messageLength & 0xFF);
    }

    ndefMessage.encode(encoded+ndefStartIndex);
    // this is always at least 1 byte copy because of terminator.
    memset(encoded+ndefStartIndex+messageLength,0,bufferSize-ndefStartIndex-messageLength);
    encoded[ndefStartIndex+messageLength] = 0xFE; // terminator

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Encoded Message length %d", messageLength);
        pn532Logger.debug("Tag Capacity %d", tagCapacity);
    // nfc->PrintHex(encoded,bufferSize);
    #endif

    while (position < bufferSize) { //bufferSize is always times pagesize so no "last chunk" check
        // write page
        if (controller->mifareultralightWritePage(page, src) != HMS_PN532_OK)
            return HMS_PN532_ERROR;
		#if HMS_PN532_DEBUG_ENABLED
            pn532Logger.debug("Wrote page %d - ", page);
    	// nfc->PrintHex(src,ULTRALIGHT_PAGE_SIZE);
    	#endif
        page++;
        src+=MIFAREULTRALIGHT_PAGE_SIZE;
        position+=MIFAREULTRALIGHT_PAGE_SIZE;
    }
    return HMS_PN532_OK;
}