#ifndef HMS_PN532_NDEF_RECORD_H
#define HMS_PN532_NDEF_RECORD_H

#include "HMS_PN532_Config.h"

typedef enum {
    HMS_PN532_NDEF_TNF_EMPTY            = 0x0,
    HMS_PN532_NDEF_TNF_UNKNOWN          = 0x05,
    HMS_PN532_NDEF_TNF_RESERVED         = 0x07,
    HMS_PN532_NDEF_TNF_UNCHANGED        = 0x06,
    HMS_PN532_NDEF_TNF_WELL_KNOWN       = 0x01,
    HMS_PN532_NDEF_TNF_MIME_MEDIA       = 0x02,
    HMS_PN532_NDEF_TNF_ABSOLUTE_URI     = 0x03,
    HMS_PN532_NDEF_TNF_EXTERNAL_TYPE    = 0x04
} HMS_PN532_NDEF_TNF_TypeDef;

class HMS_PN532_NDEF_Record {
    public:
        HMS_PN532_NDEF_Record();
        ~HMS_PN532_NDEF_Record();
        HMS_PN532_NDEF_Record(const HMS_PN532_NDEF_Record& rhs);

        HMS_PN532_NDEF_Record& operator=(const HMS_PN532_NDEF_Record& rhs);
   
        std::string getId();
        int getEncodedSize();
        std::string getType();


        void setTnf(byte tnf)                   { this->tnf = tnf;                                  }

        
        byte getTnf() const                     { return tnf;                                       }
        void getId(byte *id)                    { memcpy(id, this->id, idLength);                   }
        void getType(byte *type)                { memcpy(type, this->type, typeLength);             }
        int getPayloadLength() const            { return payloadLength;                             }
        void getPayload(byte *payload)          { memcpy(payload, this->payload, payloadLength);    }
        unsigned int getIdLength()  const       { return idLength;                                  }
        unsigned int getTypeLength() const      { return typeLength;                                }


        void setId(const byte *id, const unsigned int numBytes);
        void setPayload(const byte *payload, const int numBytes);
        void setType(const byte *type, const unsigned int numBytes);

        void print();
        void encode(byte *data, bool firstRecord, bool lastRecord);
    private:
        int             payloadLength;
        byte            *id;
        byte            tnf;                                            // 3 bit
        byte            *type;
        byte            *payload;
        unsigned int    idLength;
        unsigned int    typeLength;

        byte getTnfByte(bool firstRecord, bool lastRecord);
};

#endif // HMS_PN532_NDEF_RECORD_H