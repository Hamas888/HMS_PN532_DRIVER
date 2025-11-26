#ifndef HMS_PN532_CONTROLLER_H
#define HMS_PN532_CONTROLLER_H

#include "HMS_PN532_Config.h"
#include "HMS_PN532_ComInterface.h"

class HMS_PN532_Controller {
public:
    HMS_PN532_Controller(HMS_PN532_Interface &interface);
    ~HMS_PN532_Controller();

    void begin();
    uint32_t getFirmwareVersion();
    HMS_PN532_StatusTypeDef samConfig();
    HMS_PN532_StatusTypeDef tgGetData(uint8_t *buf, uint8_t len);
    HMS_PN532_StatusTypeDef inRelease(const uint8_t relevantTarget = 0);
    HMS_PN532_StatusTypeDef tgSetData(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);

    uint8_t readGPIO();
    HMS_PN532_StatusTypeDef writeGPIO(uint8_t pinstate);
    

    uint32_t readRegister(uint16_t registerAddress);
    uint32_t writeRegister(uint16_t registerAddress, uint8_t value);

    
    HMS_PN532_StatusTypeDef felicaRelease();
    HMS_PN532_StatusTypeDef felicaRequestResponse(uint8_t * mode);
    HMS_PN532_StatusTypeDef felicaRequestSystemCode(uint8_t *numSystemCode, uint16_t *systemCodeList);
    HMS_PN532_StatusTypeDef felicaRequestService(uint8_t numNode, uint16_t *nodeCodeList, uint16_t *keyVersions);
    HMS_PN532_StatusTypeDef felicaSendCommand (const uint8_t * command, uint8_t commandlength, uint8_t * response, uint8_t * responseLength);
    HMS_PN532_StatusTypeDef felicaPolling(uint16_t systemCode, uint8_t requestCode, uint8_t *idm, uint8_t *pmm, uint16_t *systemCodeResponse, uint16_t timeout=1000);
    HMS_PN532_StatusTypeDef felicaReadWithoutEncryption (uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16]);
    HMS_PN532_StatusTypeDef felicaWriteWithoutEncryption (uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16]);
    
    HMS_PN532_StatusTypeDef setRFField(uint8_t autoRFCA, uint8_t rFOnOff);
    HMS_PN532_StatusTypeDef setPassiveActivationRetries(uint8_t maxRetries);

    HMS_PN532_StatusTypeDef tgInitAsTarget(uint16_t timeout = 0);
    HMS_PN532_StatusTypeDef tgInitAsTarget(const uint8_t* command, const uint8_t len, const uint16_t timeout = 0);

    

    // ISO14443A functions
    HMS_PN532_StatusTypeDef inListPassiveTarget();
    HMS_PN532_StatusTypeDef readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid, uint8_t &uidLength, uint16_t timeout = 1000);
    HMS_PN532_StatusTypeDef inDataExchange(uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength);

    // Mifare Classic functions
    HMS_PN532_StatusTypeDef mifareclassicIsFirstBlock (uint32_t uiBlock);
    HMS_PN532_StatusTypeDef mifareclassicIsTrailerBlock (uint32_t uiBlock);
    HMS_PN532_StatusTypeDef mifareclassicAuthenticateBlock (uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData);
    HMS_PN532_StatusTypeDef mifareclassicReadDataBlock (uint8_t blockNumber, uint8_t *data);
    HMS_PN532_StatusTypeDef mifareclassicWriteDataBlock (uint8_t blockNumber, uint8_t *data);
    HMS_PN532_StatusTypeDef mifareclassicFormatNDEF ();
    HMS_PN532_StatusTypeDef mifareclassicWriteNDEFURI (uint8_t sectorNumber, uint8_t uriIdentifier, const char *url);

    // Mifare Ultralight functions
    HMS_PN532_StatusTypeDef mifareultralightReadPage (uint8_t page, uint8_t *buffer);
    HMS_PN532_StatusTypeDef mifareultralightWritePage (uint8_t page, uint8_t *buffer);

    uint8_t *getBuffer(uint8_t *len) {
        *len = sizeof(pn532_packetbuffer) - 4;
        return pn532_packetbuffer;
    };

private:
    uint8_t             uid[7];                                         // ISO14443A uid
    uint8_t             uidLen;                                         // uid len
    uint8_t             key[6];                                         // Mifare Classic key
    uint8_t             inListedTag;                                    // Tg number of inlisted tag.
    uint8_t             felicaIDm[8];                                   // FeliCa IDm (NFCID2)
    uint8_t             felicaPMm[8];                                   // FeliCa PMm (PAD)
    uint8_t             pn532_packetbuffer[64];
    HMS_PN532_Interface *interface              = nullptr;
};

#endif // HMS_PN532_CONTROLLER_H