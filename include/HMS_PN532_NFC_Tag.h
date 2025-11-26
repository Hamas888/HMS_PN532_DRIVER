#ifndef HMS_PN532_NFC_TAG_H
#define HMS_PN532_NFC_TAG_H

#include "HMS_PN532_Config.h"
#include "HMS_PN532_NDEF_Message.h"

class HMS_PN532_NFC_Tag {
    public:
        HMS_PN532_NFC_Tag();
        HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength);
        HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength, std::string tagType);
        HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength, std::string tagType, HMS_PN532_NDEF_Message& ndefMessage);
        HMS_PN532_NFC_Tag(byte *uid, unsigned int uidLength, std::string tagType, const byte *ndefData, const int ndefDataLength);

        ~HMS_PN532_NFC_Tag();

        HMS_PN532_NFC_Tag& operator=(const HMS_PN532_NFC_Tag& rhs);
        
        bool hasNdefMessage() const                     { return (ndefMessage != NULL);                                                         }
        uint8_t getUidLength() const                    {   return uidLength;                                                                   }
        std::string getTagType() const                  {   return tagType;                                                                     }
        HMS_PN532_NDEF_Message getNdefMessage() const   {   return *ndefMessage;                                                                }  
        void getUid(byte *uid, unsigned int uidLength)  {   memcpy(uid, this->uid, this->uidLength < uidLength ? this->uidLength : uidLength);  }

        void print();
        std::string getUidString();

    private:
        byte                        *uid;
        std::string                 tagType;                                    // Mifare Classic, NFC Forum Type {1,2,3,4}, Unknown
        unsigned int                uidLength;
        HMS_PN532_NDEF_Message      *ndefMessage;
};

#endif // HMS_PN532_NFC_TAG_H