#ifndef HMS_PN532_NDEF_MESSAGE_H
#define HMS_PN532_NDEF_MESSAGE_H

#include "HMS_PN532_Config.h"
#include "HMS_PN532_NDEF_Record.h"

class HMS_PN532_NDEF_Message {
    public:
        HMS_PN532_NDEF_Message();
        ~HMS_PN532_NDEF_Message();
        HMS_PN532_NDEF_Message(const HMS_PN532_NDEF_Message& rhs);
        HMS_PN532_NDEF_Message(const byte *data, const int numBytes);

        HMS_PN532_NDEF_Message& operator=(const HMS_PN532_NDEF_Message& rhs);

        int getEncodedSize();                                                               // need so we can pass array to encode
        void encode(uint8_t *data);

        HMS_PN532_StatusTypeDef addEmptyRecord();
        HMS_PN532_StatusTypeDef addUriRecord(std::string uri);
        HMS_PN532_StatusTypeDef addTextRecord(std::string text);
        HMS_PN532_StatusTypeDef addRecord(HMS_PN532_NDEF_Record& record);
        HMS_PN532_StatusTypeDef addTextRecord(std::string text, std::string encoding);
        HMS_PN532_StatusTypeDef addMimeMediaRecord(std::string mimeType, std::string payload);
        HMS_PN532_StatusTypeDef addMimeMediaRecord(std::string mimeType, uint8_t *payload, int payloadLength);

        void print();
        unsigned int getRecordCount() const                 { return recordCount;                                                                    }
        HMS_PN532_NDEF_Record getRecord(int index) const    { return (index > -1 && index < recordCount) ? records[index] : HMS_PN532_NDEF_Record(); }
        HMS_PN532_NDEF_Record operator[](int index) const   { return getRecord(index);                                                               }
    private:
        unsigned int            recordCount;
        HMS_PN532_NDEF_Record   records[HMS_PN532_MAX_NDEF_RECORDS];
};

#endif // HMS_PN532_NDEF_MESSAGE_H