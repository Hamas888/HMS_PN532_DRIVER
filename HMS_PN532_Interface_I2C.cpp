#include "HMS_PN532_Interface_I2C.h"


#if defined(HMS_PLATFORM_DESKTOP)
HMS_PN532_Interface_I2C::HMS_PN532_Interface_I2C(const char* device, uint8_t addr) : pn532_device(device) {
    // Constructor implementation for Desktop platform
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::init() {
    return HMS_PN532_OK;
}
#elif defined(HMS_PN532_PLATFORM_ZEPHYR)
HMS_PN532_Interface_I2C::HMS_PN532_Interface_I2C(const struct device *i2c_dev, uint8_t addr) 
    : pn532_i2c_dev(const_cast<struct device *>(i2c_dev)) {
    // Constructor implementation for Zephyr platform
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::init() {
    return HMS_PN532_OK;
}
#elif defined(HMS_PN532_PLATFORM_ESP_IDF)
HMS_PN532_Interface_I2C::HMS_PN532_Interface_I2C(i2c_port_t i2c_port, uint8_t addr) 
    : pn532_i2c_port(i2c_port), pn532_i2c_dev(nullptr) {
    // Constructor implementation for ESP-IDF platform
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::init() {
    return HMS_PN532_OK;
}
#elif defined(HMS_PN532_PLATFORM_STM32_HAL)
HMS_PN532_Interface_I2C::HMS_PN532_Interface_I2C( I2C_HandleTypeDef *hi2c, uint8_t addr) : pn532_hi2c(hi2c) {
    // Constructor implementation for STM32 HAL platform
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::init() {
    return HMS_PN532_OK;
}
#elif defined(HMS_PN532_PLATFORM_ARDUINO) && (defined(HMS_PN532_ARDUINO_ESP32) || defined(HMS_PN532_ARDUINO_ESP8266))
HMS_PN532_Interface_I2C::HMS_PN532_Interface_I2C(TwoWire *theWire, uint8_t addr) : pn532_wire(theWire), deviceAddress(addr) {
    command = 0;
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::init() {
    if (!pn532_wire) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("I2C device is NULL");
    #endif
        return HMS_PN532_ERROR;
    }

    pn532_wire->begin(HMS_PN532_I2C_SDA_PIN, HMS_PN532_I2C_SCL_PIN, HMS_PN532_I2C_CLOCK_SPEED);
    pn532_wire->beginTransmission(deviceAddress);

    if (pn532_wire->endTransmission() != 0) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Device not found at address 0x%02X", deviceAddress);
    #endif
        return HMS_PN532_NOT_FOUND;
    }

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.info("I2C device initialized at address 0x%02X", deviceAddress);
    #endif

    return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::readACKFrame() {
    static const uint8_t ACK_FRAME[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    uint8_t ackResp[sizeof(ACK_FRAME)];
    uint16_t time = 0;

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Waiting for ACK frame...");
    #endif

    while (true) {
        if (pn532_wire->requestFrom(deviceAddress, sizeof(ACK_FRAME) + 1)) {
            if (pn532_wire->read() & 1) break;
        }
        pn532Delay(1);
        if (++time > HMS_PN532_ACK_WAIT_TIME) {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.warn("ACK wait timeout");
            #endif
            return HMS_PN532_TIMEOUT;
        }
    }

    for (uint8_t i = 0; i < sizeof(ACK_FRAME); i++) ackResp[i] = pn532_wire->read();

    if (memcmp(ackResp, ACK_FRAME, sizeof(ACK_FRAME)) != 0) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Invalid ACK frame");
        #endif
        return HMS_PN532_INVALID_ACK;
    }

    return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::getResponseLength(uint8_t &length, uint16_t timeoutMs) {
    static const uint8_t NACK_FRAME[] = {0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    uint16_t time = 0;

    while (true) {
        if (pn532_wire->requestFrom(deviceAddress, sizeof(NACK_FRAME))) {
            if (pn532_wire->read() & 1) break;
        }
        pn532Delay(1);
        if (++time > timeoutMs && timeoutMs != 0) {
            #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.warn("Timeout getting response length");
            #endif
            return HMS_PN532_TIMEOUT;
        }
    }

    if (pn532_wire->read() != HMS_PN532_PREAMBLE ||
        pn532_wire->read() != HMS_PN532_STARTCODE1 ||
        pn532_wire->read() != HMS_PN532_STARTCODE2) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Invalid frame header");
        #endif
        return HMS_PN532_INVALID_FRAME;
    }

    length = pn532_wire->read();

    pn532_wire->beginTransmission(deviceAddress);                                                                               // Send request for last respond msg again
    for (uint8_t i = 0; i < sizeof(NACK_FRAME); ++i)
        pn532_wire->write(NACK_FRAME[i]);
    pn532_wire->endTransmission();

    return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::read(uint8_t *buffer, uint8_t len, uint16_t timeoutMs) {
    uint8_t resLen = 0;
    return read(buffer, len, resLen, timeoutMs);
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::read(uint8_t *buffer, uint8_t len, uint8_t &resLen, uint16_t timeoutMs) {
    uint16_t time = 0;
    uint8_t length = 0;

    HMS_PN532_StatusTypeDef status = getResponseLength(length, timeoutMs);
    if (status != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Failed to get response length status code: %d", status);
        #endif
        return status;
    }

    do {                                                                                                                        // Wait for PN532 ready
        if (pn532_wire->requestFrom(deviceAddress, (size_t)(6 + length + 2))) {
            if (pn532_wire->read() & 1)
                break;
        }
        pn532Delay(1);
        if (++time > timeoutMs && timeoutMs != 0)
            return HMS_PN532_TIMEOUT;
    } while (true);


    if (pn532_wire->read() != HMS_PN532_PREAMBLE   ||                                                                          // Validate frame header
        pn532_wire->read() != HMS_PN532_STARTCODE1 ||
        pn532_wire->read() != HMS_PN532_STARTCODE2
    )   return HMS_PN532_INVALID_FRAME;

    length = pn532_wire->read();

    if ((uint8_t)(length + pn532_wire->read()) != 0) return HMS_PN532_INVALID_FRAME;

    uint8_t tfi = pn532_wire->read();
    uint8_t cmd = pn532_wire->read();

    if (tfi != HMS_PN532_PN532TOHOST || cmd != (uint8_t)(command + 1)) return HMS_PN532_INVALID_FRAME;

    length -= 2;
    if (length > len) return HMS_PN532_NO_SPACE;

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Frame CMD: %02X, DataLen: %u", cmd, length);
    #endif

    uint8_t sum = tfi + cmd;                                                                                                   // Read payload
    #if HMS_PN532_DEBUG_ENABLED
        char line[128];
        size_t pos = 0;
    #endif

    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = pn532_wire->read();
        sum += buffer[i];
        #if HMS_PN532_DEBUG_ENABLED
            if (pos < sizeof(line) - 4) pos += snprintf(line + pos, sizeof(line) - pos, "%02X ", buffer[i]);
        #endif
    }

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Payload (%u bytes): %s", length, length > 0? line : "(empty)");
    #endif

    uint8_t checksum = pn532_wire->read();
    if ((uint8_t)(sum + checksum) != 0) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Checksum mismatch (sum=0x%02X, chk=0x%02X)", sum, checksum);
        #endif
        return HMS_PN532_INVALID_FRAME;
    }

    pn532_wire->read();                                                                                                        // POSTAMBLE discard
    resLen = length;
    return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::write(const uint8_t *header, uint8_t headerLen, const uint8_t *body, uint8_t bodyLen) {
    command = header[0];
    pn532_wire->beginTransmission(deviceAddress);

    pn532_wire->write(HMS_PN532_PREAMBLE);
    pn532_wire->write(HMS_PN532_STARTCODE1);
    pn532_wire->write(HMS_PN532_STARTCODE2);

    uint8_t length = headerLen + bodyLen + 1;
    pn532_wire->write(length);
    pn532_wire->write((uint8_t)(~length + 1));

    pn532_wire->write(HMS_PN532_HOSTTOPN532);
    uint8_t sum = HMS_PN532_HOSTTOPN532;

    auto writeBytes = [&](const uint8_t *data, uint8_t len, const char *label) -> bool {
        #if HMS_PN532_DEBUG_ENABLED
            char line[128];
            size_t pos = 0;
        #endif
        for (uint8_t i = 0; i < len; i++) {
            if (!pn532_wire->write(data[i])) {
                #if HMS_PN532_DEBUG_ENABLED
                    pn532Logger.error("Write overflow in %s", label);
                #endif
                return false;
            }
            sum += data[i];
            #if HMS_PN532_DEBUG_ENABLED
                if (pos < sizeof(line) - 4) pos += snprintf(line + pos, sizeof(line) - pos, "%02X ", data[i]);
            #endif
        }
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.debug("%s (%u bytes): %s", label, len, pos ? line : "(empty)");
        #endif
        return true;
    };

    if (!writeBytes(header, headerLen, "Header"))
        return HMS_PN532_INVALID_FRAME;
    if (!writeBytes(body, bodyLen, "Body"))
        return HMS_PN532_INVALID_FRAME;

    uint8_t checksum = (uint8_t)(~sum + 1);
    pn532_wire->write(checksum);
    pn532_wire->write(HMS_PN532_POSTAMBLE);
    pn532_wire->endTransmission();

    return readACKFrame();
}
#endif

HMS_PN532_StatusTypeDef HMS_PN532_Interface_I2C::wakeup() {
    pn532Delay(500);
    return HMS_PN532_OK;
}