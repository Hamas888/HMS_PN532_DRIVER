#include "HMS_PN532_NDEF_Record.h"

HMS_PN532_NDEF_Record::HMS_PN532_NDEF_Record() {
    tnf             = 0;
    typeLength      = 0;
    payloadLength   = 0;
    idLength        = 0;
    type            = (byte *)NULL;
    payload         = (byte *)NULL;
    id              = (byte *)NULL;
}

HMS_PN532_NDEF_Record::~HMS_PN532_NDEF_Record() {
    if (idLength)       free(id);
    if (typeLength)     free(type);
    if (payloadLength)  free(payload);
}

HMS_PN532_NDEF_Record::HMS_PN532_NDEF_Record(const HMS_PN532_NDEF_Record& rhs) {
    tnf             = rhs.tnf;
    typeLength      = rhs.typeLength;
    payloadLength   = rhs.payloadLength;
    idLength        = rhs.idLength;
    type            = (byte *)NULL;
    payload         = (byte *)NULL;
    id              = (byte *)NULL;

    if (idLength)       {   id      = (byte*)malloc(idLength);          memcpy(id, rhs.id, idLength);                }
    if (typeLength)     {   type    = (byte*)malloc(typeLength);        memcpy(type, rhs.type, typeLength);          }
    if (payloadLength)  {   payload = (byte*)malloc(payloadLength);     memcpy(payload, rhs.payload, payloadLength); }
}

HMS_PN532_NDEF_Record& HMS_PN532_NDEF_Record::operator=(const HMS_PN532_NDEF_Record& rhs) {
    if (this != &rhs) {
        if (idLength)       free(id);
        if (typeLength)     free(type);
        if (payloadLength)  free(payload);

        tnf             = rhs.tnf;
        typeLength      = rhs.typeLength;
        payloadLength   = rhs.payloadLength;
        idLength        = rhs.idLength;

        if (idLength)       {   id      = (byte*)malloc(idLength);          memcpy(id, rhs.id, idLength);                }
        if (typeLength)     {   type    = (byte*)malloc(typeLength);        memcpy(type, rhs.type, typeLength);          }
        if (payloadLength)  {   payload = (byte*)malloc(payloadLength);     memcpy(payload, rhs.payload, payloadLength); }
    }
    return *this;
}

int HMS_PN532_NDEF_Record::getEncodedSize() {
    int size = 2; // tnf + typeLength
    if (payloadLength > 0xFF) {
        size += 4;
    } else {
        size += 1;
    }

    if (idLength) {
        size += 1;
    }

    size += (typeLength + payloadLength + idLength);

    return size;
}

std::string HMS_PN532_NDEF_Record::getId() {
    char id[idLength + 1];
    memcpy(id, this->id, idLength);
    id[idLength] = '\0'; // null terminate
    return std::string(id);
}

std::string HMS_PN532_NDEF_Record::getType() {
    char type[typeLength + 1];
    memcpy(type, this->type, typeLength);
    type[typeLength] = '\0'; // null terminate
    return std::string(type);
}

byte HMS_PN532_NDEF_Record::getTnfByte(bool firstRecord, bool lastRecord) {
    int value = tnf;

    if (firstRecord) { // mb
        value = value | 0x80;
    }

    if (lastRecord) { //
        value = value | 0x40;
    }

    // chunked flag is always false for now
    // if (cf) {
    //     value = value | 0x20;
    // }

    if (payloadLength <= 0xFF) {
        value = value | 0x10;
    }

    if (idLength) {
        value = value | 0x8;
    }

    return value;
}

void HMS_PN532_NDEF_Record::encode(byte *data, bool firstRecord, bool lastRecord) {
    uint8_t* data_ptr = &data[0];

    *data_ptr = getTnfByte(firstRecord, lastRecord);
    data_ptr += 1;

    *data_ptr = typeLength;
    data_ptr += 1;

    if (payloadLength <= 0xFF) {  // short record
        *data_ptr = payloadLength;
        data_ptr += 1;
    } else { // long format
        // 4 bytes but we store length as an int
        data_ptr[0] = 0x0; // (payloadLength >> 24) & 0xFF;
        data_ptr[1] = 0x0; // (payloadLength >> 16) & 0xFF;
        data_ptr[2] = (payloadLength >> 8) & 0xFF;
        data_ptr[3] = payloadLength & 0xFF;
        data_ptr += 4;
    }

    if (idLength) {
        *data_ptr = idLength;
        data_ptr += 1;
    }

    memcpy(data_ptr, type, typeLength);
    data_ptr += typeLength;

    memcpy(data_ptr, payload, payloadLength);
    data_ptr += payloadLength;

    if (idLength) {
        memcpy(data_ptr, id, idLength);
        data_ptr += idLength;
    }
}

void HMS_PN532_NDEF_Record::setId(const byte * id, const unsigned int numBytes) {
    if (this->idLength) {
        free(this->id);
    }

    this->id = (byte*)malloc(numBytes);
    memcpy(this->id, id, numBytes);
    this->idLength = numBytes;
}

void HMS_PN532_NDEF_Record::setType(const byte * type, const unsigned int numBytes) {
    if (this->typeLength) {
        free(this->type);
    }

    this->type = (uint8_t*)malloc(numBytes);
    memcpy(this->type, type, numBytes);
    this->typeLength = numBytes;
}

void HMS_PN532_NDEF_Record::setPayload(const byte * payload, const int numBytes) {
    if (payloadLength) {
        free(this->payload);
    }

    this->payload = (byte*)malloc(numBytes);
    memcpy(this->payload, payload, numBytes);
    this->payloadLength = numBytes;
}

void HMS_PN532_NDEF_Record::print() {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("NDEF Record");
        pn532Logger.debug("TNF 0x%20x", tnf);

        switch (tnf) {
            case HMS_PN532_NDEF_TNF_EMPTY:
                pn532Logger.debug("Empty");
                break;
            case HMS_PN532_NDEF_TNF_WELL_KNOWN:
                pn532Logger.debug("Well Known");
                break;
            case HMS_PN532_NDEF_TNF_MIME_MEDIA:
                pn532Logger.debug("Mime Media");
                break;
            case HMS_PN532_NDEF_TNF_ABSOLUTE_URI:
                pn532Logger.debug("Absolute URI");
                break;
            case HMS_PN532_NDEF_TNF_EXTERNAL_TYPE:
                pn532Logger.debug("External");
                break;
            case HMS_PN532_NDEF_TNF_UNKNOWN:
                pn532Logger.debug("Unknown");
                break;
            case HMS_PN532_NDEF_TNF_UNCHANGED:
                pn532Logger.debug("Unchanged");
                break;
            case HMS_PN532_NDEF_TNF_RESERVED:
                pn532Logger.debug("Reserved");
                break;
            default:
                pn532Logger.debug("Unknown TNF");
        }
        pn532Logger.debug("Type Length 0x%02x", typeLength);
        pn532Logger.debug("Payload Length 0x%02x", payloadLength);
        if (idLength) {
            pn532Logger.debug("Id Length 0x%02x", idLength);
        }
        
        if (typeLength > 0) {
            char* typeStr = (char*)malloc(typeLength + 1);
            if (typeStr) {
                memcpy(typeStr, type, typeLength);
                typeStr[typeLength] = '\0';
                pn532Logger.debug("Type: %s", typeStr);
                free(typeStr);
            }
        }

        if (payloadLength > 0) {
            char* payloadStr = (char*)malloc(payloadLength * 3 + 1);
            if (payloadStr) {
                char* ptr = payloadStr;
                for (int i = 0; i < payloadLength; i++) {
                    ptr += sprintf(ptr, "%02X ", payload[i]);
                }
                pn532Logger.debug("Payload: %s", payloadStr);
                free(payloadStr);
            }
        }

        if (idLength > 0) {
             char* idStr = (char*)malloc(idLength * 3 + 1);
             if (idStr) {
                char* ptr = idStr;
                for (int i = 0; i < idLength; i++) {
                    ptr += sprintf(ptr, "%02X ", id[i]);
                }
                pn532Logger.debug("Id: %s", idStr);
                free(idStr);
             }
        }
        pn532Logger.debug("Record is %d bytes", getEncodedSize());
    #endif
}
