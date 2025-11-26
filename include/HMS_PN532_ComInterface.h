#ifndef HMS_PN532_COMINTERFACE_H
#define HMS_PN532_COMINTERFACE_H

#include "HMS_PN532_Config.h"

class HMS_PN532_Interface {
    public:
        void pn532Delay(uint32_t ms) {

            #if defined(HMS_PLATFORM_DESKTOP)
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            #elif defined(HMS_PN532_PLATFORM_ZEPHYR)
                k_msleep(ms);
            #elif defined(HMS_PN532_PLATFORM_STM32_HAL)
                HAL_Delay(ms);
            #elif defined(HMS_PN532_PLATFORM_ARDUINO) || defined(HMS_PN532_ARDUINO_ESP8266)
                delay(ms);
            #elif defined(HMS_PN532_PLATFORM_ESP_IDF) || (defined(HMS_PN532_PLATFORM_ARDUINO) && defined(HMS_PN532_ARDUINO_ESP32))
                vTaskDelay(ms / portTICK_PERIOD_MS);
            #endif
        }

        virtual HMS_PN532_StatusTypeDef init() = 0;
        virtual HMS_PN532_StatusTypeDef wakeup() = 0;

        virtual HMS_PN532_StatusTypeDef read(
            uint8_t* buffer, uint8_t len, uint16_t timeoutMs = 1000
        ) = 0;

        virtual HMS_PN532_StatusTypeDef read(
            uint8_t* buffer, uint8_t len, uint8_t &resLen, uint16_t timeoutMs = 1000
        ) = 0;
        
        virtual HMS_PN532_StatusTypeDef write(
            const uint8_t *header, uint8_t headerLen, const uint8_t *body = 0, uint8_t bodyLen = 0
        ) = 0;
};

#endif // HMS_PN532_COMINTERFACE_H