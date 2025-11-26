/*
 ============================================================================================================================================
 * File:        HMS_PN532_Config.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Oct 23 2025
 * Brief:       This Package Provide PN532 Driver Configurations
 * 
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

#ifndef HMS_PN532_CONFIG_H
#define HMS_PN532_CONFIG_H

#if defined(ARDUINO)                                                                   // Platform detection
  #include <Wire.h>  
  #include <Arduino.h>
  #if defined(ESP32)
    #include <freertos/task.h>
    #include <freertos/FreeRTOS.h>
    #define HMS_PN532_ARDUINO_ESP32
  #elif defined(ESP8266)
    // Include ESP8266 PN532 libraries here
    #define HMS_PN532_ARDUINO_ESP8266
  #endif
  #define HMS_PN532_PLATFORM_ARDUINO
#elif defined(ESP_PLATFORM)
  // ESP-IDF specific includes
  #define HMS_PLATFORM_ESP_IDF
#elif defined(__ZEPHYR__)
  // Zephyr specific includes
  #define HMS_PLATFORM_ZEPHYR
#elif defined(__STM32__)
  // STM32 HAL specific includes
  #define HMS_PLATFORM_STM32_HAL
#elif defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
  // Desktop specific includes
  // #define HMS_PLATFORM_DESKTOP
#endif // Platform detection

#define HMS_PN532_SPI                                   0x01                           // SPI Communication Interface
#define HMS_PN532_I2C                                   0x02                           // I2C Communication Interface
#define HMS_PN532_UART                                  0x03                           // UART Communication Interface

#ifndef HMS_PN532_COM_INTERFACE
  #define HMS_PN532_COM_INTERFACE                       HMS_PN532_I2C                  // Define the communication interface
#endif

#if HMS_PN532_COM_INTERFACE == HMS_PN532_I2C                                           // Required for Arduino / ESP32 / ESP8266 platforms
  #ifndef HMS_PN532_I2C_SDA_PIN
    #define HMS_PN532_I2C_SDA_PIN                       8                              // I2C SDA Pin
  #endif
  #ifndef HMS_PN532_I2C_SCL_PIN
    #define HMS_PN532_I2C_SCL_PIN                       9                              // I2C SCL Pin
  #endif
  #ifndef HMS_PN532_I2C_CLOCK_SPEED
    #define HMS_PN532_I2C_CLOCK_SPEED                   10000                          // I2C Clock Speed
  #endif
#elif HMS_PN532_COM_INTERFACE == HMS_PN532_SPI
  #ifndef HMS_PN532_SPI_SCK_PIN
    #define HMS_PN532_SPI_SCK_PIN                       18                             // SPI SCK Pin
  #endif
  #ifndef HMS_PN532_SPI_MOSI_PIN
    #define HMS_PN532_SPI_MOSI_PIN                      23                             // SPI MOSI Pin
  #endif
  #ifndef HMS_PN532_SPI_MISO_PIN
    #define HMS_PN532_SPI_MISO_PIN                      19                             // SPI MISO Pin
  #endif
  #ifndef HMS_PN532_SPI_SS_PIN
    #define HMS_PN532_SPI_SS_PIN                        5                              // SPI SS Pin
  #endif
  #ifndef HMS_PN532_SPI_CLOCK_SPEED
    #define HMS_PN532_SPI_CLOCK_SPEED                   1000000                        // SPI Clock Speed
  #endif
#elif HMS_PN532_COM_INTERFACE == HMS_PN532_UART
  #ifndef HMS_PN532_UART_RX_PIN
    #define HMS_PN532_UART_RX_PIN                       16                             // UART RX Pin
  #endif
  #ifndef HMS_PN532_UART_TX_PIN
    #define HMS_PN532_UART_TX_PIN                       17                             // UART TX Pin
  #endif
  #ifndef HMS_PN532_UART_BAUDRATE
    #define HMS_PN532_UART_BAUDRATE                     115200                         // UART Baudrate
  #endif
#endif // HMS_PN532_COM_INTERFACE

/*
  ┌─────────────────────────────────────────────────────────────────────┐
  │ Note:     Enable only if ChronoLog is included                      │
  │ Requires: ChronoLog library → https://github.com/Hamas888/ChronoLog │
  └─────────────────────────────────────────────────────────────────────┘
*/
#ifndef HMS_PN532_DEBUG_ENABLED
  #define HMS_PN532_DEBUG_ENABLED                       0                             // Enable debug messages (1=enabled, 0=disabled)
#endif

#if defined(HMS_PN532_DEBUG_ENABLED) && (HMS_PN532_DEBUG_ENABLED == 1)
  #if __has_include("ChronoLog.h")
    #include "ChronoLog.h"
    #ifndef HMS_PN532_LOG_LEVEL
      #define HMS_PN532_LOG_LEVEL                       CHRONOLOG_LEVEL_DEBUG
    #endif
    extern ChronoLogger pn532Logger;
  #else
    #error "HMS_PN532_DEBUG_ENABLED is enabled but ChronoLog.h is missing. Please include the https://github.com/Hamas888/ChronoLog in your project."
  #endif
#endif


#define HMS_PN532_DEVICE_NAME                           "PN532"                       // Device Name
#define HMS_PN532_DEVICE_ADDR                           (0x48 >> 1)                   // PN532 default i2c address w/ AD0 high

// PN532 Commands
// ======================================================
// =============== PN532 SYSTEM & GENERAL ===============
// ======================================================
#define HMS_PN532_COMMAND_DIAGNOSE                      0x00
#define HMS_PN532_COMMAND_GETFIRMWAREVERSION            0x02
#define HMS_PN532_COMMAND_GETGENERALSTATUS              0x04
#define HMS_PN532_COMMAND_READREGISTER                  0x06
#define HMS_PN532_COMMAND_WRITEREGISTER                 0x08
#define HMS_PN532_COMMAND_READGPIO                      0x0C
#define HMS_PN532_COMMAND_WRITEGPIO                     0x0E
#define HMS_PN532_COMMAND_SETSERIALBAUDRATE             0x10
#define HMS_PN532_COMMAND_SETPARAMETERS                 0x12
#define HMS_PN532_COMMAND_SAMCONFIGURATION              0x14
#define HMS_PN532_COMMAND_POWERDOWN                     0x16

// ======================================================
// =============== PN532 RF CONFIGURATION ===============
// ======================================================
#define HMS_PN532_COMMAND_RFCONFIGURATION               0x32
#define HMS_PN532_COMMAND_RFREGULATIONTEST              0x58

// ======================================================
// ============= INITIATOR MODE COMMANDS ================
// ======================================================
#define HMS_PN532_COMMAND_INDATAEXCHANGE                0x40
#define HMS_PN532_COMMAND_INCOMMUNICATETHRU             0x42
#define HMS_PN532_COMMAND_INDESELECT                    0x44
#define HMS_PN532_COMMAND_INJUMPFORPSL                  0x46
#define HMS_PN532_COMMAND_INLISTPASSIVETARGET           0x4A
#define HMS_PN532_COMMAND_INATR                         0x50
#define HMS_PN532_COMMAND_INPSL                         0x4E
#define HMS_PN532_COMMAND_INRELEASE                     0x52
#define HMS_PN532_COMMAND_INSELECT                      0x54
#define HMS_PN532_COMMAND_INJUMPFORDEP                  0x56
#define HMS_PN532_COMMAND_INAUTOPOLL                    0x60

// ======================================================
// =============== TARGET MODE COMMANDS =================
// ======================================================
#define HMS_PN532_COMMAND_TGINITASTARGET                0x8C
#define HMS_PN532_COMMAND_TGGETDATA                     0x86
#define HMS_PN532_COMMAND_TGGETINITIATORCOMMAND         0x88
#define HMS_PN532_COMMAND_TGGETTARGETSTATUS             0x8A
#define HMS_PN532_COMMAND_TGRESPONSETOINITIATOR         0x90
#define HMS_PN532_COMMAND_TGSETDATA                     0x8E
#define HMS_PN532_COMMAND_TGSETGENERALBYTES             0x92
#define HMS_PN532_COMMAND_TGSETMETADATA                 0x94

// ======================================================
// ================= RESPONSE CODES =====================
// ======================================================
#define HMS_PN532_RESPONSE_INDATAEXCHANGE               0x41
#define HMS_PN532_RESPONSE_INLISTPASSIVETARGET          0x4B

// ======================================================
// ================= PROTOCOL TYPES =====================
// ======================================================
#define HMS_PN532_MIFARE_ISO14443A                      0x00


// ======================================================
// ================== MIFARE COMMANDS ===================
// ======================================================
#define HMS_PN532_MIFARE_CMD_AUTH_A                     0x60                          // Authenticate with Key A
#define HMS_PN532_MIFARE_CMD_AUTH_B                     0x61                          // Authenticate with Key B
#define HMS_PN532_MIFARE_CMD_READ                       0x30                          // Read 16 bytes from a block
#define HMS_PN532_MIFARE_CMD_WRITE                      0xA0                          // Write 16 bytes to a block
#define HMS_PN532_MIFARE_CMD_WRITE_ULTRALIGHT           0xA2                          // Write 4 bytes (MIFARE Ultralight)
#define HMS_PN532_MIFARE_CMD_TRANSFER                   0xB0                          // Transfer value from internal register to block
#define HMS_PN532_MIFARE_CMD_DECREMENT                  0xC0                          // Decrement value block
#define HMS_PN532_MIFARE_CMD_INCREMENT                  0xC1                          // Increment value block
#define HMS_PN532_MIFARE_CMD_STORE                      0xC2                          // Store data into value block

// ======================================================
// ================== FELICA COMMANDS ===================
// ======================================================
#define HMS_PN532_FELICA_CMD_POLLING                    0x00                          // Request system to respond with IDm/PMm
#define HMS_PN532_FELICA_CMD_REQUEST_SERVICE            0x02                          // Check available services
#define HMS_PN532_FELICA_CMD_REQUEST_RESPONSE           0x04                          // Request system response code
#define HMS_PN532_FELICA_CMD_READ_WITHOUT_ENCRYPTION    0x06                          // Read data without encryption
#define HMS_PN532_FELICA_CMD_WRITE_WITHOUT_ENCRYPTION   0x08                          // Write data without encryption
#define HMS_PN532_FELICA_CMD_REQUEST_SYSTEM_CODE        0x0C                          // Request system codes from card


// Prefixes for NDEF Records (to identify record type)
// ======================================================
// ================ PN532 NDEF URI PREFIXES ==============
// ======================================================
#define HMS_PN532_NDEF_URIPREFIX_NONE                   0x00                          // No prefix
#define HMS_PN532_NDEF_URIPREFIX_HTTP_WWWDOT            0x01                          // "http://www."
#define HMS_PN532_NDEF_URIPREFIX_HTTPS_WWWDOT           0x02                          // "https://www."
#define HMS_PN532_NDEF_URIPREFIX_HTTP                   0x03                          // "http://"
#define HMS_PN532_NDEF_URIPREFIX_HTTPS                  0x04                          // "https://"
#define HMS_PN532_NDEF_URIPREFIX_TEL                    0x05                          // "tel:"
#define HMS_PN532_NDEF_URIPREFIX_MAILTO                 0x06                          // "mailto:"
#define HMS_PN532_NDEF_URIPREFIX_FTP_ANONAT             0x07                          // "ftp://anonymous:anonymous@"
#define HMS_PN532_NDEF_URIPREFIX_FTP_FTPDOT             0x08                          // "ftp://ftp."
#define HMS_PN532_NDEF_URIPREFIX_FTPS                   0x09                          // "ftps://"
#define HMS_PN532_NDEF_URIPREFIX_SFTP                   0x0A                          // "sftp://"
#define HMS_PN532_NDEF_URIPREFIX_SMB                    0x0B                          // "smb://"
#define HMS_PN532_NDEF_URIPREFIX_NFS                    0x0C                          // "nfs://"
#define HMS_PN532_NDEF_URIPREFIX_FTP                    0x0D                          // "ftp://"
#define HMS_PN532_NDEF_URIPREFIX_DAV                    0x0E                          // "dav://"
#define HMS_PN532_NDEF_URIPREFIX_NEWS                   0x0F                          // "news:"
#define HMS_PN532_NDEF_URIPREFIX_TELNET                 0x10                          // "telnet://"
#define HMS_PN532_NDEF_URIPREFIX_IMAP                   0x11                          // "imap:"
#define HMS_PN532_NDEF_URIPREFIX_RTSP                   0x12                          // "rtsp://"
#define HMS_PN532_NDEF_URIPREFIX_URN                    0x13                          // "urn:"
#define HMS_PN532_NDEF_URIPREFIX_POP                    0x14                          // "pop:"
#define HMS_PN532_NDEF_URIPREFIX_SIP                    0x15                          // "sip:"
#define HMS_PN532_NDEF_URIPREFIX_SIPS                   0x16                          // "sips:"
#define HMS_PN532_NDEF_URIPREFIX_TFTP                   0x17                          // "tftp:"
#define HMS_PN532_NDEF_URIPREFIX_BTSPP                  0x18                          // "btspp://"
#define HMS_PN532_NDEF_URIPREFIX_BTL2CAP                0x19                          // "btl2cap://"
#define HMS_PN532_NDEF_URIPREFIX_BTGOEP                 0x1A                          // "btgoep://"
#define HMS_PN532_NDEF_URIPREFIX_TCPOBEX                0x1B                          // "tcpobex://"
#define HMS_PN532_NDEF_URIPREFIX_IRDAOBEX               0x1C                          // "irdaobex://"
#define HMS_PN532_NDEF_URIPREFIX_FILE                   0x1D                          // "file://"
#define HMS_PN532_NDEF_URIPREFIX_URN_EPC_ID             0x1E                          // "urn:epc:id:"
#define HMS_PN532_NDEF_URIPREFIX_URN_EPC_TAG            0x1F                          // "urn:epc:tag:"
#define HMS_PN532_NDEF_URIPREFIX_URN_EPC_PAT            0x20                          // "urn:epc:pat:"
#define HMS_PN532_NDEF_URIPREFIX_URN_EPC_RAW            0x21                          // "urn:epc:raw:"
#define HMS_PN532_NDEF_URIPREFIX_URN_EPC                0x22                          // "urn:epc:"
#define HMS_PN532_NDEF_URIPREFIX_URN_NFC                0x23                          // "urn:nfc:"


// ======================================================
// ================ PN532 GPIO DEFINITIONS ===============
// ======================================================
#define HMS_PN532_GPIO_VALIDATIONBIT                    0x80                          // Validation bit mask
#define HMS_PN532_GPIO_P30                              0x00                          // GPIO pin P30
#define HMS_PN532_GPIO_P31                              0x01                          // GPIO pin P31
#define HMS_PN532_GPIO_P32                              0x02                          // GPIO pin P32
#define HMS_PN532_GPIO_P33                              0x03                          // GPIO pin P33
#define HMS_PN532_GPIO_P34                              0x04                          // GPIO pin P34
#define HMS_PN532_GPIO_P35                              0x05                          // GPIO pin P35

// ======================================================
// ================== FELICA CONSTANTS ==================
// ======================================================
#define HMS_PN532_FELICA_READ_MAX_SERVICE_NUM           16                            // Max number of services for read
#define HMS_PN532_FELICA_READ_MAX_BLOCK_NUM             12                            // Max blocks per read (typical FeliCa card)
#define HMS_PN532_FELICA_WRITE_MAX_SERVICE_NUM          16                            // Max number of services for write
#define HMS_PN532_FELICA_WRITE_MAX_BLOCK_NUM            10                            // Max blocks per write (typical FeliCa card)
#define HMS_PN532_FELICA_REQ_SERVICE_MAX_NODE_NUM       32                            // Max nodes per Request Service

// ======================================================
// ================== NDEF RTD TYPE DEFINES ==============
// ======================================================
#define HMS_PN532_NDEF_RTD_TEXT                         0x54                          // 'T' - Well-known type for Text records
#define HMS_PN532_NDEF_RTD_URI                          0x55                          // 'U' - Well-known type for URI records
#define HMS_PN532_NDEF_RTD_SMART_POSTER                 0x53                          // 'S' - Well-known type for Smart Poster records
#define HMS_PN532_NDEF_RTD_ALTERNATIVE_CARRIER          0x61                          // 'a' - Alternative carrier record
#define HMS_PN532_NDEF_RTD_HANDOVER_CARRIER             0x48                          // 'H' - Handover carrier record
#define HMS_PN532_NDEF_RTD_HANDOVER_REQUEST             0x68                          // 'h' - Handover request record
#define HMS_PN532_NDEF_RTD_HANDOVER_SELECT              0x73                          // 's' - Handover select record

#define HMS_PN532_PREAMBLE                              0x00                          // Preamble byte
#define HMS_PN532_POSTAMBLE                             0x00                          // Postamble byte
#define HMS_PN532_STARTCODE1                            0x00                          // Start code 1 byte
#define HMS_PN532_STARTCODE2                            0xFF                          // Start code 2 byte

#define HMS_PN532_HOSTTOPN532                           0xD4                          // Host to PN532 direction byte
#define HMS_PN532_PN532TOHOST                           0xD5                          // PN532 to Host direction byte

#define HMS_PN532_ACK_WAIT_TIME                         10                            // ms, timeout of waiting for ACK

#define HMS_REVERSE_BITS_ORDER(b)                       \
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4; \
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2; \
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;                                              // Macro to reverse bit order in a byte 
  
#define HMS_PN532_MAX_NDEF_RECORDS                      4                             // Max NDEF records in a message 
#define HMS_PN532_MAX_CARD_NUM_SCAN                     1                             // Max number of cards to scan


typedef enum {
  HMS_PN532_OK              = 0x00,
  HMS_PN532_BUSY            = 0x01,
  HMS_PN532_ERROR           = 0x02,
  HMS_PN532_TIMEOUT         = -0x02,
  HMS_PN532_NOT_FOUND       = 0x04,
  HMS_PN532_NO_SPACE        = -0x04,
  HMS_PN532_INVALID_ACK     = -0x01,
  HMS_PN532_INVALID_FRAME   = -0x03,
  HMS_PN532_INVALID_COMMAND = -0x05
} HMS_PN532_StatusTypeDef;

#endif // HMS_PN532_CONFIG_H