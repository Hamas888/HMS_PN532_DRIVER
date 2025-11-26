#include <Arduino.h>
#include "ChronoLog.h"
#include "HMS_PN532_DRIVER.h"

HMS_PN532    nfcReader;
ChronoLogger logger("test");

void setup() {
    Serial.begin(115200);
    if(nfcReader.begin() != HMS_PN532_OK) {
        logger.error("Failed to initialize NFC reader");
        while(1);
    }
}

void loop() {
    static unsigned long lastUpdate = 0;
    
    if(millis() - lastUpdate > 5000) {
        lastUpdate = millis();
        if(nfcReader.tagAvailable(1000) == HMS_PN532_OK) {
            HMS_PN532_NFC_Tag tag = nfcReader.readTag();
            
            String      uidStr  = "";
            uint8_t     uidLen  = nfcReader.getUidLength();
            uint8_t*    uid     = nfcReader.getUid();

            for (uint8_t i = 0; i < uidLen; i++) {
                if (uid[i] < 0x10) uidStr += "0";
                uidStr += String(uid[i], HEX);
            }

            uidStr.toUpperCase();

            logger.info("Tag detected with UID: %s", uidStr.c_str());
            logger.info("Tag Type: %s", tag.getTagType().c_str());

            if(tag.hasNdefMessage()) {
                HMS_PN532_NDEF_Message ndefMessage = tag.getNdefMessage();

                logger.info(
                    "NDEF Message with %d %s found", ndefMessage.getRecordCount(), 
                    ndefMessage.getRecordCount() == 1 ? "record" : "records"
                );

                for(int i = 0; i < ndefMessage.getRecordCount(); i++) {
                    HMS_PN532_NDEF_Record record = ndefMessage.getRecord(i);

                    logger.info(
                        "Record %d: Type: %s, Payload Length: %d", i+1, 
                        record.getType().c_str(), record.getPayloadLength()
                    );
                    
                    int     payloadLen  = record.getPayloadLength();
                    byte*   payload     = new byte[payloadLen + 1];

                    record.getPayload(payload);
                    payload[payloadLen] = '\0';
                    
                    String payloadStr = "";
                    int startIdx = 0;
                    
                    if (record.getType() == "U" && payloadLen > 0) {                                    // Check if it's a URI record (Type "U")
                        byte prefix = payload[0];
                        startIdx = 1;                                                                   // Skip the prefix byte
                        switch(prefix) {
                            case 0x00:                                              break;              // No prefix
                            case 0x01: payloadStr += "http://www.";                 break;
                            case 0x02: payloadStr += "https://www.";                break;
                            case 0x03: payloadStr += "http://";                     break;
                            case 0x04: payloadStr += "https://";                    break;
                            case 0x05: payloadStr += "tel:";                        break;
                            case 0x06: payloadStr += "mailto:";                     break;
                            case 0x07: payloadStr += "ftp://anonymous:anonymous@";  break;
                            case 0x08: payloadStr += "ftp://ftp.";                  break;
                            case 0x09: payloadStr += "ftps://";                     break;
                            case 0x0A: payloadStr += "sftp://";                     break;
                            default:   startIdx    = 0;                             break;              // Unknown prefix, just treat as data
                        }
                    } else if (record.getType() == "T" && payloadLen > 0) {                             // Check if it's a Text record (Type "T")
                        byte status = payload[0];
                        int languageCodeLength = status & 0x3F;                                         // Lower 6 bits
                        startIdx = 1 + languageCodeLength;                                              // Skip status byte and language code
                    }

                    for(int k=startIdx; k<payloadLen; k++) {
                        if (payload[k] >= 32 && payload[k] <= 126) {
                            payloadStr += (char)payload[k];
                        } else {
                            payloadStr += '.'; 
                        }
                    }
                    logger.info("Record %d Payload: %s\n", i+1, payloadStr.c_str());
                    delete[] payload;
                }
                tag.print();
            } else {
                logger.warn("No NDEF message found on the tag\n");
            }
        } else {
            logger.warn("No tag detected\n");
        }
    }

}