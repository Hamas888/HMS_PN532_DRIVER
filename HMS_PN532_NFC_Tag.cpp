#include "HMS_PN532_NFC_Tag.h"


HMS_PN532_NFC_Tag::HMS_PN532_NFC_Tag() {
    uid         = 0;
    uidLength   = 0;
    tagType     = "Unknown";
    ndefMessage = (HMS_PN532_NDEF_Message*)NULL;
}

HMS_PN532_NFC_Tag::~HMS_PN532_NFC_Tag() {
    delete ndefMessage;
}

HMS_PN532_NFC_Tag::HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength) {
    this->uid           = uid;
    this->uidLength     = uidLength;
    this->tagType       = "Unknown";
    this->ndefMessage   = (HMS_PN532_NDEF_Message*)NULL;
}

HMS_PN532_NFC_Tag::HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength, std::string tagType) {
    this->uid           = uid;
    this->uidLength     = uidLength;
    this->tagType       = tagType;
    this->ndefMessage   = (HMS_PN532_NDEF_Message*)NULL;
}

HMS_PN532_NFC_Tag::HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength, std::string tagType, HMS_PN532_NDEF_Message& ndefMessage) {
    this->uid           = uid;
    this->uidLength     = uidLength;
    this->tagType       = tagType;
    this->ndefMessage   = new HMS_PN532_NDEF_Message(ndefMessage);
}

HMS_PN532_NFC_Tag::HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength, std::string tagType, const byte *ndefData, const int ndefDataLength) {
    this->uid           = uid;
    this->uidLength     = uidLength;
    this->tagType       = tagType;
    this->ndefMessage   = new HMS_PN532_NDEF_Message(ndefData, ndefDataLength);
}

HMS_PN532_NFC_Tag &HMS_PN532_NFC_Tag::operator=(const HMS_PN532_NFC_Tag& rhs) {
    if (this != &rhs) {
        delete ndefMessage;
        uid = rhs.uid;
        uidLength = rhs.uidLength;
        tagType = rhs.tagType;
        ndefMessage = rhs.ndefMessage;
    }
    return *this;
}

void HMS_PN532_NFC_Tag::print() {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("NFC Tag - %s", tagType.c_str());
        pn532Logger.debug("UID %s", getUidString().c_str());
    if (ndefMessage == NULL) {
        pn532Logger.debug("No NDEF Message");
    } else {
        ndefMessage->print();
    }
    #endif
}

std::string HMS_PN532_NFC_Tag::getUidString() {
    std::string uidString = "";
    for (int i = 0; i < uidLength; i++) {
        if (i > 0) {
            uidString += " ";
        }

        // Convert byte to uppercase hex string
        char hexBuffer[3];
        sprintf(hexBuffer, "%02X", uid[i]);
        uidString += hexBuffer;
    }
    return uidString;
}