/*
 ============================================================================================================================================
 * File:        HMS_PN532_DRIVER.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Oct 23 2025
 * Brief:       This file provides PN532 Driver functionalities for embedded & Desktop systems (Arduino, ESP-IDF, Zephyr, STM32 HAL).
 ============================================================================================================================================
 * License: 
 * MIT License
 * 
 * Copyright (c) 2025 Hamas Saeed
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do 
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For any inquiries, contact Hamas Saeed at hamasaeed@gmail.com
 ============================================================================================================================================
 */

#ifndef HMS_PN532_DRIVER_H
#define HMS_PN532_DRIVER_H

#include "HMS_PN532_Config.h"
#include "HMS_PN532_NFC_Tag.h"
#include "HMS_PN532_Controller.h"

#include "HMS_PN532_MifareClassic.h"
#include "HMS_PN532_MifareUltralight.h"


#if (HMS_PN532_COM_INTERFACE == HMS_PN532_I2C)
  #include "HMS_PN532_Interface_I2C.h"
  extern HMS_PN532_Interface_I2C default_interface;
#elif (HMS_PN532_COM_INTERFACE == HMS_PN532_SPI)
  // #include "HMS_PN532_Interface_SPI.h"
#elif (HMS_PN532_COM_INTERFACE == HMS_PN532_UART)
  // #include "HMS_PN532_Interface_UART.h"
#else
  #error "Selected HMS_PN532_COM_INTERFACE is not supported. Please choose a valid interface."
#endif

typedef enum {
  HMS_PN532_TAG_TYPE_2,
  HMS_PN532_TAG_TYPE_UNKNOWN,
  HMS_PN532_TAG_TYPE_MIFARE_CLASSIC
} HMS_PN532_TagTypeDef;

class HMS_PN532 {
  public:
    HMS_PN532(HMS_PN532_Interface *interface = &default_interface);
    ~HMS_PN532();

    HMS_PN532_StatusTypeDef begin();
    HMS_PN532_StatusTypeDef tagAvailable(unsigned long timeout=0);

    uint8_t* getUid()                           { return uid;                }
    uint8_t  getUidLength()                     { return uidLength;          }
    uint8_t  getFirmwareVersion()               { return firmwareVersion;    }
    uint16_t getChipId()                        { return chipId;             }

    HMS_PN532_NFC_Tag readTag();
    HMS_PN532_StatusTypeDef cleanTag();
    HMS_PN532_StatusTypeDef eraseTag();
    HMS_PN532_StatusTypeDef formatTag();
    HMS_PN532_StatusTypeDef writeTag(HMS_PN532_NDEF_Message& ndefMessage);

  private:
    byte                  uid[7];                                 // Buffer to store the returned UID
    uint8_t               uidLength;                              // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    uint8_t               firmwareVersion;
    uint16_t              chipId;
    HMS_PN532_Interface   *pn532_interface = nullptr;
    HMS_PN532_Controller  *pn532_controller = nullptr;

    HMS_PN532_TagTypeDef getTagType();
};

#endif // HMS_PN532_DRIVER_H