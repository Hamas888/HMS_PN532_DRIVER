#include "HMS_PN532_DRIVER.h"

#if HMS_PN532_DEBUG_ENABLED
  ChronoLogger pn532Logger("HMS_PN532", HMS_PN532_LOG_LEVEL);
#endif

#if (HMS_PN532_COM_INTERFACE == HMS_PN532_I2C)
  #include "HMS_PN532_Interface_I2C.h"
  HMS_PN532_Interface_I2C default_interface;
#elif (HMS_PN532_COM_INTERFACE == HMS_PN532_SPI)
  // #include "HMS_PN532_Interface_SPI.h"
#elif (HMS_PN532_COM_INTERFACE == HMS_PN532_UART)
  // #include "HMS_PN532_Interface_UART.h"
#endif

HMS_PN532::HMS_PN532(HMS_PN532_Interface *interface) {
    pn532_interface = interface;
    pn532_controller = new HMS_PN532_Controller(*pn532_interface);
}

HMS_PN532::~HMS_PN532() {
    if(pn532_interface) {
        delete pn532_interface;
        pn532_controller = nullptr;
    }
}

HMS_PN532_StatusTypeDef HMS_PN532::begin() {
    pn532_controller->begin();
    uint32_t versiondata = pn532_controller->getFirmwareVersion();

    if (!versiondata) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Didn't find PN532");
        #endif
        return HMS_PN532_NOT_FOUND;
    }

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.info("Found chip PN5%2X", (versiondata>>24) & 0xFF);
        pn532Logger.info("Firmware ver. %d.%d", (versiondata>>16) & 0xFF, (versiondata>>8) & 0xFF);
    #endif

    pn532_controller->samConfig();                                                                      // configure board to read RFID tags

    return HMS_PN532_OK;
}

HMS_PN532_TagTypeDef HMS_PN532::getTagType() {
    switch(uidLength) {
        case 4:
            return HMS_PN532_TAG_TYPE_MIFARE_CLASSIC;
        case 7:
            return HMS_PN532_TAG_TYPE_2;
        default:
            return HMS_PN532_TAG_TYPE_UNKNOWN;
    }
}

HMS_PN532_StatusTypeDef HMS_PN532::tagAvailable(unsigned long timeout) {
    memset(uid, 0, sizeof(uid));

    if (timeout == 0) {
        return pn532_controller->readPassiveTargetID(HMS_PN532_MIFARE_ISO14443A, uid, uidLength);
    } else {
        return pn532_controller->readPassiveTargetID(HMS_PN532_MIFARE_ISO14443A, uid, uidLength, timeout);
    }
}

HMS_PN532_NFC_Tag HMS_PN532::readTag() {
    switch(getTagType()) {
        case HMS_PN532_TAG_TYPE_2: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.info("Card Type Mifare Ultralight");
            #endif
            HMS_PN532_MifareUltralight mifareUltralight = HMS_PN532_MifareUltralight(*pn532_controller);
            return mifareUltralight.readTag(uid, uidLength);
        }
        case HMS_PN532_TAG_TYPE_MIFARE_CLASSIC: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.info("Card Type Mifare Classic");
            #endif
            HMS_PN532_MifareClassic mifareClassic = HMS_PN532_MifareClassic(*pn532_controller);
            return mifareClassic.readTag(uid, uidLength);
        }
        default: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.warn("No driver for card type %d", getTagType());
            #endif
            return HMS_PN532_NFC_Tag(uid, uidLength);
        }
    }
}

HMS_PN532_StatusTypeDef HMS_PN532::cleanTag() {
    switch(getTagType()) {
        case HMS_PN532_TAG_TYPE_2: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.debug("Cleaning Mifare Ultralight");
            #endif
            HMS_PN532_MifareUltralight mifareUltralight = HMS_PN532_MifareUltralight(*pn532_controller);
            return mifareUltralight.cleanTag();
        }
        case HMS_PN532_TAG_TYPE_MIFARE_CLASSIC: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.debug("Cleaning Mifare Classic");
            #endif
            HMS_PN532_MifareClassic mifareClassic = HMS_PN532_MifareClassic(*pn532_controller);
            return mifareClassic.formatMifare(uid, uidLength);
        }
        default:
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("No driver for card type %d", getTagType());
            #endif
            return HMS_PN532_ERROR;
    }
}

HMS_PN532_StatusTypeDef HMS_PN532::eraseTag() {
    HMS_PN532_NDEF_Message message = HMS_PN532_NDEF_Message();
    message.addEmptyRecord();
    return writeTag(message);
}

HMS_PN532_StatusTypeDef HMS_PN532::formatTag() {
    switch(getTagType()) {
        case HMS_PN532_TAG_TYPE_MIFARE_CLASSIC: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.debug("Formatting Mifare Classic");
            #endif
            HMS_PN532_MifareClassic mifareClassic = HMS_PN532_MifareClassic(*pn532_controller);
            return mifareClassic.formatNDEF(uid, uidLength);
        }
        default: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Unsupported Tag.");
            #endif
            return HMS_PN532_ERROR;
        }
    }
}

HMS_PN532_StatusTypeDef HMS_PN532::writeTag(HMS_PN532_NDEF_Message& ndefMessage) {
    switch(getTagType()) {
        case HMS_PN532_TAG_TYPE_2: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.debug("Writing Mifare Ultralight");
            #endif
            HMS_PN532_MifareUltralight mifareUltralight = HMS_PN532_MifareUltralight(*pn532_controller);
            return mifareUltralight.writeTag(ndefMessage, uid, uidLength);
        }
        case HMS_PN532_TAG_TYPE_MIFARE_CLASSIC: {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.debug("Writing Mifare Classic");
            #endif
            HMS_PN532_MifareClassic mifareClassic = HMS_PN532_MifareClassic(*pn532_controller);
            return mifareClassic.writeTag(ndefMessage, uid, uidLength);
        }
        default:
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("No driver for card type %d", getTagType());
            #endif
            return HMS_PN532_ERROR;
    }
}