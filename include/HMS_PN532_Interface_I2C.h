#ifndef HMS_PN532_INTERFACE_I2C_H
#define HMS_PN532_INTERFACE_I2C_H

#include "HMS_PN532_ComInterface.h"

class HMS_PN532_Interface_I2C : public HMS_PN532_Interface { 
    public:
        #if defined(HMS_PLATFORM_DESKTOP)
            HMS_PN532_Interface_I2C(
                const char* device = "/dev/i2c-1", uint8_t addr = HMS_PN532_DEVICE_ADDR
            );
        #elif defined(HMS_PN532_PLATFORM_ZEPHYR)
            HMS_PN532_Interface_I2C(
                const struct device *i2c_dev = NULL, uint8_t addr = HMS_PN532_DEVICE_ADDR
            );
        #elif defined(HMS_PN532_PLATFORM_ESP_IDF)
            HMS_PN532_Interface_I2C(
                i2c_port_t i2c_port = I2C_NUM_0, uint8_t addr = HMS_PN532_DEVICE_ADDR
            );
        #elif defined(HMS_PN532_PLATFORM_STM32_HAL)
            HMS_PN532_Interface_I2C(
                I2C_HandleTypeDef *hi2c = NULL, uint8_t addr = HMS_PN532_DEVICE_ADDR
            );
        #elif defined(HMS_PN532_PLATFORM_ARDUINO) && (defined(HMS_PN532_ARDUINO_ESP32) || defined(HMS_PN532_ARDUINO_ESP8266))
            HMS_PN532_Interface_I2C(
                TwoWire *theWire = &Wire, uint8_t addr = HMS_PN532_DEVICE_ADDR
            );
        #endif
        
        HMS_PN532_StatusTypeDef init() override;
        HMS_PN532_StatusTypeDef wakeup() override;

        HMS_PN532_StatusTypeDef read(
            uint8_t* buffer, uint8_t len, uint16_t timeoutMs = 1000
        ) override;

        HMS_PN532_StatusTypeDef read(
            uint8_t* buffer, uint8_t len, uint8_t &resLen, uint16_t timeoutMs = 1000
        ) override;

        HMS_PN532_StatusTypeDef write(
            const uint8_t *header, uint8_t headerLen, const uint8_t *body = 0, uint8_t bodyLen = 0
        ) override;

    private:
        uint8_t command;
        uint8_t deviceAddress;
        #if defined(HMS_PLATFORM_DESKTOP)
            const char* pn532_device;
        #elif defined(HMS_PN532_PLATFORM_ZEPHYR)
            struct device *pn532_i2c_dev;
        #elif defined(HMS_PN532_PLATFORM_ESP_IDF)
            i2c_port_t pn532_i2c_port;
            struct device *pn532_i2c_dev;
        #elif defined(HMS_PN532_PLATFORM_STM32_HAL)
            I2C_HandleTypeDef *pn532_hi2c;
        #elif defined(HMS_PN532_PLATFORM_ARDUINO) || defined(HMS_PN532_ARDUINO_ESP32) || defined(HMS_PN532_ARDUINO_ESP8266)
            TwoWire *pn532_wire = NULL;
        #endif

        HMS_PN532_StatusTypeDef readACKFrame();
        HMS_PN532_StatusTypeDef getResponseLength(uint8_t &length, uint16_t timeoutMs = 1000);
};

#endif // HMS_PN532_INTERFACE_I2C_H