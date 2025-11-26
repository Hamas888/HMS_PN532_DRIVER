#include "HMS_PN532_NDEF_Message.h"


HMS_PN532_NDEF_Message::HMS_PN532_NDEF_Message() {
    recordCount = 0;
}

HMS_PN532_NDEF_Message::~HMS_PN532_NDEF_Message() {}

HMS_PN532_NDEF_Message::HMS_PN532_NDEF_Message(const HMS_PN532_NDEF_Message& rhs)  {
    recordCount = rhs.recordCount;
    for (int i = 0; i < recordCount; i++) {
        records[i] = rhs.records[i];
    }

}

HMS_PN532_NDEF_Message::HMS_PN532_NDEF_Message(const byte * data, const int numBytes) {
    recordCount = 0;

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Creating NDEF Message from data, %d bytes", numBytes);
        char hexBuffer[numBytes * 3 + 1];                                                                   // "FF " per byte + null terminator
        for (int i = 0; i < numBytes; i++) {
            sprintf(&hexBuffer[i * 3], "%02X ", data[i]);
        }
        pn532Logger.debug("Data (HEX): %s", hexBuffer);
    #endif
    
    for(int i = 0; i <= numBytes;) {
        byte tnf_byte   = data[i];                                                                          // Decode tnf - first byte is tnf with bit flags
        bool mb         = (tnf_byte & 0x80) != 0;                                                           // See the NFDEF spec for more info
        bool me         = (tnf_byte & 0x40) != 0;
        bool cf         = (tnf_byte & 0x20) != 0;
        bool sr         = (tnf_byte & 0x10) != 0;
        bool il         = (tnf_byte & 0x8)  != 0;
        byte tnf        = (tnf_byte & 0x7);

        HMS_PN532_NDEF_Record record = HMS_PN532_NDEF_Record();
        record.setTnf(tnf);

        i++;
        int typeLength = data[i];

        int payloadLength = 0;
        if (sr) {
            i++;
            payloadLength = data[i];
        } else {
            payloadLength = ((0xFF & data[++i]) << 24) | ((0xFF & data[++i]) << 16) | ((0xFF & data[++i]) << 8) | (0xFF & data[++i]);
        }

        int idLength = 0;
        if (il) {
            i++;
            idLength = data[i];
        }

        i++;
        record.setType(&data[i], typeLength);
        i += typeLength;

        if (il) {
            record.setId(&data[i], idLength);
            i += idLength;
        }

        record.setPayload(&data[i], payloadLength);
        i += payloadLength;

        addRecord(record);

        if (me) break;                                                                                      // last message
    }
    int index = 0;
}

HMS_PN532_NDEF_Message& HMS_PN532_NDEF_Message::operator=(const HMS_PN532_NDEF_Message& rhs) {
    if (this != &rhs) {
        for (int i = 0; i < recordCount; i++) {                                                             // delete existing records
            records[i] = HMS_PN532_NDEF_Record();                                                           // is this the right way to delete existing records?
        }

        recordCount = rhs.recordCount;
        for (int i = 0; i < recordCount; i++) {
            records[i] = rhs.records[i];
        }
    }
    return *this;
}

void HMS_PN532_NDEF_Message::print() {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("NDEF Message with %d records", recordCount);
        pn532Logger.debug("Bytes: %d", getEncodedSize());
        for (int i = 0; i < recordCount; i++) {
            pn532Logger.debug("Record %d:", i);
            records[i].print();
        }
    #endif
}

int HMS_PN532_NDEF_Message::getEncodedSize() {
    int size = 0;
    for (int i = 0; i < recordCount; i++){
        size += records[i].getEncodedSize();
    }
    return size;
}

void HMS_PN532_NDEF_Message::encode(uint8_t* data) {
    uint8_t* data_ptr = &data[0];

    for (int i = 0; i < recordCount; i++) {
        records[i].encode(data_ptr, i == 0, (i + 1) == recordCount);
        data_ptr += records[i].getEncodedSize();
    }
}

HMS_PN532_StatusTypeDef HMS_PN532_NDEF_Message::addEmptyRecord() {
    HMS_PN532_NDEF_Record r = HMS_PN532_NDEF_Record();
    r.setTnf(HMS_PN532_NDEF_TNF_EMPTY);
    if(addRecord(r) == HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.debug("Added Empty Record");
        #endif
        return HMS_PN532_OK;
    } else {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.warn("Failed to add Empty Record");
        #endif
        return HMS_PN532_ERROR;
    }
}

HMS_PN532_StatusTypeDef HMS_PN532_NDEF_Message::addUriRecord(std::string uri) {
    HMS_PN532_NDEF_Record r;
    r.setTnf(HMS_PN532_NDEF_TNF_WELL_KNOWN);

    uint8_t RTD_URI[1] = { HMS_PN532_NDEF_RTD_URI };
    r.setType(RTD_URI, sizeof(RTD_URI));

    uint8_t prefixCode = HMS_PN532_NDEF_URIPREFIX_NONE;

    if (uri.rfind("http://www.", 0) == 0)       prefixCode = HMS_PN532_NDEF_URIPREFIX_HTTP_WWWDOT;
    else if (uri.rfind("https://www.", 0) == 0) prefixCode = HMS_PN532_NDEF_URIPREFIX_HTTPS_WWWDOT;
    else if (uri.rfind("http://", 0) == 0)      prefixCode = HMS_PN532_NDEF_URIPREFIX_HTTP;
    else if (uri.rfind("https://", 0) == 0)     prefixCode = HMS_PN532_NDEF_URIPREFIX_HTTPS;

    if (prefixCode != HMS_PN532_NDEF_URIPREFIX_NONE) {                                                  // Remove prefix from the actual URI if matched
        size_t pos = uri.find("://");
        if (pos != std::string::npos) uri = uri.substr(pos + 3);                                        // skip prefix
    }

    size_t payloadLen = 1 + uri.length();
    uint8_t payload[payloadLen];
    payload[0] = prefixCode;
    memcpy(&payload[1], uri.c_str(), uri.length());

    r.setPayload(payload, payloadLen);
    if(addRecord(r) == HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.debug("Added URI Record: %s", uri.c_str());
        #endif
        return HMS_PN532_OK;
    } else {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.warn("Failed to add URI Record: %s", uri.c_str());
        #endif
        return HMS_PN532_ERROR;
    }
}

HMS_PN532_StatusTypeDef HMS_PN532_NDEF_Message::addTextRecord(std::string text) {
    return addTextRecord(text, "en");
}

HMS_PN532_StatusTypeDef HMS_PN532_NDEF_Message::addRecord(HMS_PN532_NDEF_Record& record) {
    if (recordCount < HMS_PN532_MAX_NDEF_RECORDS) {
        records[recordCount] = record;
        recordCount++;
        return HMS_PN532_OK;
    } else {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.warn("Too many records. Increase MAX_NDEF_RECORDS.");
        #endif
        return HMS_PN532_ERROR;
    }
}

HMS_PN532_StatusTypeDef HMS_PN532_NDEF_Message::addTextRecord(std::string text, std::string encoding) {
    HMS_PN532_NDEF_Record r = HMS_PN532_NDEF_Record();
    r.setTnf(HMS_PN532_NDEF_TNF_WELL_KNOWN);

    uint8_t RTD_TEXT[1] = { HMS_PN532_NDEF_RTD_TEXT };
    r.setType(RTD_TEXT, sizeof(RTD_TEXT));

    bool utf16 = false; // hardcoded (since "encoding" here is used as language code)
    uint8_t statusByte = (utf16 ? 0x80 : 0x00) | (encoding.length() & 0x3F);

    size_t payloadLen = 1 + encoding.length() + text.length();
    uint8_t payload[payloadLen];
    payload[0] = statusByte;

    memcpy(&payload[1], encoding.c_str(), encoding.length());
    memcpy(&payload[1 + encoding.length()], text.c_str(), text.length());

    r.setPayload(payload, payloadLen);

    if (addRecord(r) == HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.debug("Added Text Record: lang=%s, text=%s", encoding.c_str(), text.c_str());
        #endif
        return HMS_PN532_OK;
    } else {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.warn("Failed to add Text Record: lang=%s, text=%s", encoding.c_str(), text.c_str());
        #endif
        return HMS_PN532_ERROR;
    }
}

HMS_PN532_StatusTypeDef HMS_PN532_NDEF_Message::addMimeMediaRecord(std::string mimeType, std::string payload) {
    uint8_t payloadBytes[payload.length()];
    memcpy(payloadBytes, payload.c_str(), payload.length());

    return addMimeMediaRecord(mimeType, payloadBytes, payload.length());
}

HMS_PN532_StatusTypeDef HMS_PN532_NDEF_Message::addMimeMediaRecord(std::string mimeType, uint8_t* payload, int payloadLength) {
    HMS_PN532_NDEF_Record r;
    r.setTnf(HMS_PN532_NDEF_TNF_MIME_MEDIA);

    size_t typeLen = mimeType.length();                                                                         // Copy MIME type (e.g., "text/plain" or "image/png")
    uint8_t type[typeLen];
    memcpy(type, mimeType.c_str(), typeLen);

    r.setType(type, typeLen);
    r.setPayload(payload, payloadLength);

    if (addRecord(r) != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.warn("Failed to add MIME Media Record: %s", mimeType.c_str());
        #endif
        return HMS_PN532_ERROR;
    } else {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.debug("Added MIME Media Record: %s", mimeType.c_str());
        #endif
        return HMS_PN532_OK;
    }
}