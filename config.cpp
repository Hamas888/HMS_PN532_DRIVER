#include "config.h"

HMS_BLE              *ble       = nullptr;
HMS_PN532            *nfc       = nullptr;
ChronoLogger         *smfLogger = nullptr;
HMS_StatusLED        *ledStrip  = new HMS_StatusLED(LED_STRIP_LENGTH, LED_STRIP_TYPE, LED_STRIP_ORDER);

uint32_t             stripColor = HMS_STATUSLED_RGB888_WHITE;
HMS::JsonValue       userData;
HMS::ParseError      parseError;

TaskHandle_t         statusLEDTaskHandle = nullptr;

void nfcTask(void *parameter) {
    nfc = new HMS_PN532();
    
    if(nfc->begin() != HMS_PN532_OK) {
        SMF_LOGGER(error, "Failed to initialize NFC reader");
        uint8_t retryCount = 0;
        while(retryCount < 5) {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            if(nfc->begin() == HMS_PN532_OK) {
                SMF_LOGGER(info, "NFC reader initialized successfully on retry %d", retryCount + 1);
                return;
            }
            retryCount++;
        }
    }

    while(true) {
        userData = readNFC();
        if(userData.isObject()) {
            SMF_LOGGER(info, "NFC Data read successfully");
            std::string jsonString = HMS::serialize(userData, true);
            ble->sendData(
                NORDIC_BLE_UART_TX_UUID, 
                (const uint8_t*)jsonString.c_str(), 
                jsonString.size()
            );
            // Delay to prevent spamming if the card is held
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    } 
}

void statusLEDTask(void *parameter) {
    if (ledStrip->begin(LED_STRIP_1_PIN, RMT_CHANNEL_0) != HMS_STATUSLED_OK) {
        SMF_LOGGER(error, "Failed to initialize LED driver! line: %d file: %s", __LINE__, __FILE__);
        return;
    }

    ledStrip->clear();                                                                                              // Clear all LEDs and show
    ledStrip->show();

    for (int i = 0; i < 30; i++) {  ledStrip->setPixelColor(HMS_STATUSLED_RGB_TO_888(255, 255, 255), i);  }
    ledStrip->show();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (true) {
        breathingEffect(*ledStrip, LED_STRIP_LENGTH, stripColor, 20, 200);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay to prevent watchdog timer reset
    }
}

HMS::JsonValue readNFC() {
    if (!nfc) return HMS::JsonValue();
    
    if(nfc->tagAvailable(1000) == HMS_PN532_OK) {
        HMS_PN532_NFC_Tag tag = nfc->readTag();
        
        String      uidStr  = "";
        uint8_t     uidLen  = nfc->getUidLength();
        uint8_t*    uid     = nfc->getUid();
    
        for (uint8_t i = 0; i < uidLen; i++) {
            if (uid[i] < 0x10) uidStr += "0";
            uidStr += String(uid[i], HEX);
        }

        uidStr.toUpperCase();

        SMF_LOGGER(info ,"Tag detected with UID: %s", uidStr.c_str());
        SMF_LOGGER(info ,"Tag Type: %s", tag.getTagType().c_str());

        return readNDEFMessage(tag);
    } else return HMS::JsonValue();
}

HMS::JsonValue readNDEFMessage(HMS_PN532_NFC_Tag &tag) {
    if(tag.hasNdefMessage()) {
        HMS::JsonValue card;
        HMS_PN532_NDEF_Message ndefMessage = tag.getNdefMessage();

        SMF_LOGGER(
            info, "NDEF Message with %d %s found", 
            ndefMessage.getRecordCount(), ndefMessage.getRecordCount() == 1 ? "record" : "records"
        );

        card["uid"] = tag.getUidString();
        card["tagType"] = tag.getTagType();

        for(int i = 0; i < ndefMessage.getRecordCount(); i++) {
            HMS_PN532_NDEF_Record record = ndefMessage.getRecord(i);

            // Filter out ghost records (empty type and payload)
            if (record.getPayloadLength() == 0 && record.getType().length() == 0) {
                return HMS::JsonValue();
            }

            SMF_LOGGER(
                info, "Record %d: Type: %s, Payload Length: %d", i+1, 
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

            bool hasNonPrintable = false;
            for(int k=startIdx; k<payloadLen; k++) {
                if (payload[k] >= 32 && payload[k] <= 126) {
                    payloadStr += (char)payload[k];
                } else {
                    payloadStr += '.'; 
                    hasNonPrintable = true;
                }
            }
            SMF_LOGGER(info, "Record %d Payload: %s\n", i+1, payloadStr.c_str());
            
            // Validation: Check for incomplete JSON
            if (payloadStr.startsWith("{") && !payloadStr.endsWith("}")) {
                 delete[] payload;
                 return HMS::JsonValue();
            }

            delete[] payload;
            card["records"][i]["type"] = record.getType();
            card["records"][i]["payload"] = payloadStr.c_str();
        }
        return card;
    } else {
        SMF_LOGGER(warn, "No NDEF message found on the tag\n");
        return HMS::JsonValue();
    }
}   

void breathingEffect(HMS_StatusLED &strip , uint16_t ledCount, uint32_t color, uint8_t delayMs, uint16_t holdTimeMs) {    
    for (int i = 0; i < ledCount; i++) {
        strip.setPixelColor(color, i);
    }
    
    for (int brightness = 0; brightness <= 255; brightness += 3) {
        strip.setBrightness(brightness);
        strip.show();
        vTaskDelay(delayMs / portTICK_PERIOD_MS);
    }
    
    vTaskDelay(holdTimeMs / portTICK_PERIOD_MS);
    
    for (int brightness = 255; brightness >= 0; brightness -= 3) {
        strip.setBrightness(brightness);
        strip.show();
        vTaskDelay(delayMs / portTICK_PERIOD_MS);
    }
    
    vTaskDelay(holdTimeMs / portTICK_PERIOD_MS);
}