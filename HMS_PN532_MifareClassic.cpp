#include "HMS_PN532_MifareClassic.h"

HMS_PN532_MifareClassic::HMS_PN532_MifareClassic(HMS_PN532_Controller& controller) {
    this->controller = &controller;
}

HMS_PN532_MifareClassic::~HMS_PN532_MifareClassic() {

}

int HMS_PN532_MifareClassic::getNdefStartIndex(byte *data) {
    for (int i = 0; i < MIFARECLASSIC_BLOCK_SIZE; i++) {
        if (data[i] == 0x0) {
            // do nothing, skip
        } else if (data[i] == 0x3) {
            return i;
        } else {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Unknown TLV: 0x%02X", data[i]);
            #endif
            return -2;
        }
    }
    return -1;
}

int HMS_PN532_MifareClassic::getBufferSize(int messageLength) {
    int bufferSize = messageLength;

    if (messageLength < 0xFF) {    // TLV header is 2 or 4 bytes, TLV terminator is 1 byte.
        bufferSize += MIFARECLASSIC_SHORT_TLV_SIZE + 1;
    } else {
        bufferSize += MIFARECLASSIC_LONG_TLV_SIZE + 1;
    }

    if (bufferSize % MIFARECLASSIC_BLOCK_SIZE != 0) {    // bufferSize needs to be a multiple of BLOCK_SIZE
        bufferSize = ((bufferSize / MIFARECLASSIC_BLOCK_SIZE) + 1) * MIFARECLASSIC_BLOCK_SIZE;
    }
    return bufferSize;
}

bool HMS_PN532_MifareClassic::decodeTLV(byte *data, int &messageLength, int &messageStartIndex) {
    int i = getNdefStartIndex(data);

    if (i < 0 || data[i] != 0x3) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("No NDEF TLV found.");
        #endif
        return false;
    } else {
        if (data[i+1] == 0xFF) {
            messageLength = ((0xFF & data[i+2]) << 8) | (0xFF & data[i+3]);
            messageStartIndex = i + MIFARECLASSIC_LONG_TLV_SIZE;
        } else {
            messageLength = data[i+1];
            messageStartIndex = i + MIFARECLASSIC_SHORT_TLV_SIZE;
        }
    }
    return true;
}

HMS_PN532_NFC_Tag HMS_PN532_MifareClassic::readTag(byte *uid, uint8_t uidLength) {
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
    int currentBlock = 4;
    int messageStartIndex = 0;
    int messageLength = 0;
    byte data[MIFARECLASSIC_BLOCK_SIZE];

    if (controller->mifareclassicAuthenticateBlock(uid, uidLength, currentBlock, 0, key) == HMS_PN532_OK) {    // read first block to get message length
        if (controller->mifareclassicReadDataBlock(currentBlock, data) == HMS_PN532_OK) {
            if (!decodeTLV(data, messageLength, messageStartIndex)) {
                return HMS_PN532_NFC_Tag(uid, uidLength, "ERROR");
            }
        } else {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Error. Failed to read block %d", currentBlock);
            #endif
            return HMS_PN532_NFC_Tag(uid, uidLength, MIFARECLASSIC_TYPE_NAME);
        }
    } else {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Error. Failed to authenticate block %d", currentBlock);
        #endif
        return HMS_PN532_NFC_Tag(uid, uidLength, MIFARECLASSIC_TYPE_NAME);
    }

    int index = 0;
    int bufferSize = getBufferSize(messageLength);
    uint8_t buffer[bufferSize];
    memset(buffer, 0, bufferSize); // Initialize buffer with zeros

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.info("Reading NDEF message...");
        pn532Logger.info("Message Length %d", messageLength);
        pn532Logger.info("Buffer Size %d", bufferSize);
    #endif

    while (index < bufferSize) {
        if (controller->mifareclassicIsFirstBlock(currentBlock) == HMS_PN532_OK) {
            if (controller->mifareclassicAuthenticateBlock(uid, uidLength, currentBlock, 0, key) != HMS_PN532_OK) {
                #if HMS_PN532_DEBUG_ENABLED
                    pn532Logger.error("Error. Sector Authentication failed for block %d", currentBlock);
                #endif
            }
        }

        if (controller->mifareclassicReadDataBlock(currentBlock, &buffer[index]) == HMS_PN532_OK) {
            #if HMS_PN532_DEBUG_ENABLED
                char hexString[MIFARECLASSIC_BLOCK_SIZE*3 + 1] = {0};
                char* ptr = hexString;
                for (int i = 0; i < MIFARECLASSIC_BLOCK_SIZE; i++) {
                    ptr += sprintf(ptr, "%02X ", buffer[index + i]);
                }
                pn532Logger.debug("Data: %s", hexString);
            #endif
        }
        else {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Error. Failed read block %d", currentBlock);
            #endif
        }

        index += MIFARECLASSIC_BLOCK_SIZE;
        currentBlock++;
        
        if (controller->mifareclassicIsTrailerBlock(currentBlock) == HMS_PN532_OK) {
            #if HMS_PN532_DEBUG_ENABLED
                 pn532Logger.info("Skipping trailer block %d", currentBlock);
            #endif
            currentBlock++;
        }
    }

    return HMS_PN532_NFC_Tag(uid, uidLength, MIFARECLASSIC_TYPE_NAME, &buffer[messageStartIndex], messageLength);
}

HMS_PN532_StatusTypeDef HMS_PN532_MifareClassic::formatNDEF(byte *uid, uint8_t uidLength) {
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t emptyNdefMesg[16] = {0x03, 0x03, 0xD0, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer0[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    HMS_PN532_StatusTypeDef status = (HMS_PN532_StatusTypeDef)controller->mifareclassicAuthenticateBlock (uid, uidLength, 0, 0, keya);
    if (status != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Unable to authenticate block 0 to enable card formatting!");
        #endif
        return HMS_PN532_ERROR;
    }
    status = (HMS_PN532_StatusTypeDef)controller->mifareclassicFormatNDEF();
    if (status != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Unable to format the card for NDEF");
        #endif
    } else {
        for (int i = 4; i < 64; i += 4) {
            status = (HMS_PN532_StatusTypeDef)controller->mifareclassicAuthenticateBlock (uid, uidLength, i, 0, keya);
            if (status == HMS_PN532_OK) {
                if (i == 4)  {// special handling for block 4
                    if (controller->mifareclassicWriteDataBlock (i, emptyNdefMesg) != HMS_PN532_OK) {
                        #if HMS_PN532_DEBUG_ENABLED
                            pn532Logger.error("Unable to write block %d", i);
                        #endif
                    }
                } else{
                    if (controller->mifareclassicWriteDataBlock(i, sectorbuffer0) != HMS_PN532_OK) {
                        #if HMS_PN532_DEBUG_ENABLED
                            pn532Logger.error("Unable to write block %d", i);
                        #endif
                    }
                }
                if (controller->mifareclassicWriteDataBlock (i+1, sectorbuffer0) != HMS_PN532_OK) {
                    #if HMS_PN532_DEBUG_ENABLED
                        pn532Logger.error("Unable to write block %d", i+1);
                    #endif
                }
                if (controller->mifareclassicWriteDataBlock (i+2, sectorbuffer0) != HMS_PN532_OK) {
                    #if HMS_PN532_DEBUG_ENABLED
                        pn532Logger.error("Unable to write block %d", i+2);
                    #endif
                }
                if (controller->mifareclassicWriteDataBlock (i+3, sectorbuffer4) != HMS_PN532_OK) {
                    #if HMS_PN532_DEBUG_ENABLED
                        pn532Logger.error("Unable to write block %d", i+3);
                    #endif
                }
            } else {
                #if HMS_PN532_DEBUG_ENABLED
                    pn532Logger.error("Unable to authenticate block %d", i);
                #endif
                controller->readPassiveTargetID(HMS_PN532_MIFARE_ISO14443A, uid, uidLength);
            }
        }
    }
    return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_MifareClassic::formatMifare(byte *uid, uint8_t uidLength) {
    uint8_t KEY_DEFAULT_KEYAB[6]     = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    uint8_t blockBuffer[16];                                                                    // Buffer to store block contents
    uint8_t blankAccessBits[3]       = { 0xff, 0x07, 0x80 };
    uint8_t idx                      = 0;
    uint8_t numOfSector              = 16;                                                      // Assume Mifare Classic 1K for now (16 4-block sectors)

    for (idx = 0; idx < numOfSector; idx++) {
        if (                                                                                    // Step 1: Authenticate the current sector using key B 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
            controller->mifareclassicAuthenticateBlock (
                uid, uidLength, MIFARECLASSIC_BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), 1, (uint8_t *)KEY_DEFAULT_KEYAB
            ) != HMS_PN532_OK
        ) {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Authentication failed for sector %d", idx);
            #endif
            return HMS_PN532_ERROR;
        }

        if (idx == 0) {                                                                         // Step 2: Write to the other blocks
            memset(blockBuffer, 0, sizeof(blockBuffer));
            if (
                controller->mifareclassicWriteDataBlock(
                    (MIFARECLASSIC_BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer
                ) != HMS_PN532_OK
            ) {
                #if HMS_PN532_DEBUG_ENABLED
                    pn532Logger.error("Unable to write to sector %d", idx);
                #endif
            }
        } else {
            memset(blockBuffer, 0, sizeof(blockBuffer));

            if (                                                                                // this block has not to be overwritten for block 0. It contains Tag id and other unique data.
                controller->mifareclassicWriteDataBlock(
                    (MIFARECLASSIC_BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer
                ) != HMS_PN532_OK
            ) {
                #if HMS_PN532_DEBUG_ENABLED
                    pn532Logger.error("Unable to write to sector %d", idx);
                #endif
            }

            if (
                controller->mifareclassicWriteDataBlock(
                    (MIFARECLASSIC_BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer
                ) != HMS_PN532_OK
            ) {
                #if HMS_PN532_DEBUG_ENABLED
                    pn532Logger.error("Unable to write to sector %d", idx);
                #endif
            }
        }

        memset(blockBuffer, 0, sizeof(blockBuffer));

        if (
            controller->mifareclassicWriteDataBlock(
                (MIFARECLASSIC_BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 1, blockBuffer
            ) != HMS_PN532_OK
        ) {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Unable to write to sector %d", idx);
            #endif
        }


        memcpy(blockBuffer, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));                      // Step 3: Reset both keys to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
        memcpy(blockBuffer + 6, blankAccessBits, sizeof(blankAccessBits));
        blockBuffer[9] = 0x69;
        memcpy(blockBuffer + 10, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));

        if (                                                                                    // Step 4: Write the trailer block
            controller->mifareclassicWriteDataBlock(
                (MIFARECLASSIC_BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), blockBuffer
            ) != HMS_PN532_OK
        )
        {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Unable to write trailer block of sector %d", idx);
            #endif
        }
    }
    return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_MifareClassic::writeTag(HMS_PN532_NDEF_Message &ndefMessage, byte *uid, uint8_t uidLength) {
    uint8_t encoded[ndefMessage.getEncodedSize()];
    ndefMessage.encode(encoded);

    uint8_t buffer[getBufferSize(sizeof(encoded))];
    memset(buffer, 0, sizeof(buffer));

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Encoded size: %d", sizeof(encoded));
        pn532Logger.debug("Buffer size: %d", sizeof(buffer));
    #endif

    if (sizeof(encoded) < 0xFF) {
        buffer[0] = 0x3;
        buffer[1] = sizeof(encoded);
        memcpy(&buffer[2], encoded, sizeof(encoded));
        buffer[2+sizeof(encoded)] = 0xFE; // terminator
    } else {
        buffer[0] = 0x3;
        buffer[1] = 0xFF;
        buffer[2] = ((sizeof(encoded) >> 8) & 0xFF);
        buffer[3] = (sizeof(encoded) & 0xFF);
        memcpy(&buffer[4], encoded, sizeof(encoded));
        buffer[4+sizeof(encoded)] = 0xFE; // terminator
    }

    // Write to tag
    int index = 0;
    int currentBlock = 4;
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 }; // this is Sector 1 - 15 key

    while (index < sizeof(buffer)) {
        if (controller->mifareclassicIsFirstBlock(currentBlock)){
            if (
                controller->mifareclassicAuthenticateBlock(
                    uid, uidLength, currentBlock, 0, key
                ) != HMS_PN532_OK
            ) {
                #if HMS_PN532_DEBUG_ENABLED
                    pn532Logger.error("Block Authentication failed for %d", currentBlock);
                #endif
                return HMS_PN532_ERROR;
            }
        }

        if (
            controller->mifareclassicWriteDataBlock(
                currentBlock, &buffer[index]
            ) == HMS_PN532_OK
        ) {
            #if HMS_PN532_DEBUG_ENABLED
                // pn532Logger.debug("Wrote block %d", currentBlock);
                // for (int i = 0; i < MIFARECLASSIC_BLOCK_SIZE; i++) {
                //     ptr += sprintf(ptr, "%02X ", buffer[i]);
                // }
                // pn532Logger.debug("Data: %s", hexString);
            #endif
        } else {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Write failed for block %d", currentBlock);
            #endif
            return HMS_PN532_ERROR;
        }
        index += MIFARECLASSIC_BLOCK_SIZE;
        currentBlock++;

        if (controller->mifareclassicIsTrailerBlock(currentBlock) == HMS_PN532_OK) {                                   // can't write to trailer block
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.debug("Skipping trailer block %d", currentBlock);
            #endif
            currentBlock++;
        }

    }
    return HMS_PN532_OK;
}