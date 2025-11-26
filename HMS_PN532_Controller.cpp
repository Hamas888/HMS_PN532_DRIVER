#include "HMS_PN532_Controller.h"

HMS_PN532_Controller::HMS_PN532_Controller(HMS_PN532_Interface &interface) : interface(&interface) {}

HMS_PN532_Controller::~HMS_PN532_Controller() {}

void HMS_PN532_Controller::begin() {
  interface->init();
  interface->wakeup();
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareultralightReadPage (uint8_t page, uint8_t *buffer) {
    if (page >= 64) {
      #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Page value out of range");
      #endif
        return HMS_PN532_ERROR;
    }

    /* Prepare the command */
    pn532_packetbuffer[0] = HMS_PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                   /* Card number */
    pn532_packetbuffer[2] = HMS_PN532_MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

    /* Send the command */
    if (interface->write(pn532_packetbuffer, 4) != HMS_PN532_OK) {
        return HMS_PN532_ERROR;
    }

    /* Read the response packet */
    interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] == 0x00) {
        /* Copy the 4 data bytes to the output buffer         */
        /* Block content starts at byte 9 of a valid response */
        /* Note that the command actually reads 16 bytes or 4  */
        /* pages at a time ... we simply discard the last 12  */
        /* bytes                                              */
        memcpy (buffer, pn532_packetbuffer + 1, 4);
    } else {
        return HMS_PN532_ERROR;
    }

    // Return OK signal
    return HMS_PN532_OK;
}


HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareultralightWritePage (uint8_t page, uint8_t *buffer) {
  /* Prepare the first command */
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = 1;                           /* Card number */
  pn532_packetbuffer[2] = HMS_PN532_MIFARE_CMD_WRITE_ULTRALIGHT; /* Mifare UL Write cmd = 0xA2 */
  pn532_packetbuffer[3] = page;                        /* page Number (0..63) */
  memcpy (pn532_packetbuffer + 4, buffer, 4);          /* Data Payload */

  /* Send the command */
  if (interface->write(pn532_packetbuffer, 8) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  /* Read the response packet */
  return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}


uint32_t HMS_PN532_Controller::getFirmwareVersion() {
    uint32_t response;

    pn532_packetbuffer[0] = HMS_PN532_COMMAND_GETFIRMWAREVERSION;

    if (interface->write(pn532_packetbuffer, 1) != HMS_PN532_OK) {
        return 0;
    }

    if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer)) != HMS_PN532_OK) {  // read data packet
      return 0;
    }

    response = pn532_packetbuffer[0];
    response <<= 8;
    response |= pn532_packetbuffer[1];
    response <<= 8;
    response |= pn532_packetbuffer[2];
    response <<= 8;
    response |= pn532_packetbuffer[3];

    return response;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::samConfig() {
    pn532_packetbuffer[0] = HMS_PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01;                       // normal mode;
    pn532_packetbuffer[2] = 0x14;                       // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01;                       // use IRQ pin!

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Configurating SAM");
    #endif

    if (interface->write(pn532_packetbuffer, 4) != HMS_PN532_OK)
        return HMS_PN532_ERROR;

    return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}

uint8_t HMS_PN532_Controller::readGPIO() {
    pn532_packetbuffer[0] = HMS_PN532_COMMAND_READGPIO;

    // Send the READGPIO command (0x0C)
    if (interface->write(pn532_packetbuffer, 1) != HMS_PN532_OK)
        return 0x0;

    if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer)) != HMS_PN532_OK)
        return 0x0;
   /* READGPIO response without prefix and suffix should be in the following format:

      byte            Description
      -------------   ------------------------------------------
      b0              P3 GPIO Pins
      b1              P7 GPIO Pins (not used ... taken by I2C)
      b2              Interface Mode Pins (not used ... bus select pins)
    */

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("P3 GPIO: 0x%02X", pn532_packetbuffer[0]);
        pn532Logger.debug("P7 GPIO: 0x%02X", pn532_packetbuffer[1]);
        pn532Logger.debug("I0I1 GPIO: 0x%02X", pn532_packetbuffer[2]);
    #endif

    return pn532_packetbuffer[0];
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::writeGPIO(uint8_t pinstate) {
    // Make sure pinstate does not try to toggle P32 or P34
    pinstate |= (1 << HMS_PN532_GPIO_P32) | (1 << HMS_PN532_GPIO_P34);

    // Fill command buffer
    pn532_packetbuffer[0] = HMS_PN532_COMMAND_WRITEGPIO;
    pn532_packetbuffer[1] = HMS_PN532_GPIO_VALIDATIONBIT | pinstate;  // P3 Pins
    pn532_packetbuffer[2] = 0x00;    // P7 GPIO Pins (not used ... taken by I2C)

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Writing P3 GPIO: 0x%02X", pn532_packetbuffer[1]);
    #endif

    // Send the WRITEGPIO command (0x0E)
    if (interface->write(pn532_packetbuffer, 3) != HMS_PN532_OK)
        return HMS_PN532_ERROR;

    return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}

uint32_t HMS_PN532_Controller::readRegister(uint16_t registerAddress) {
    uint32_t response;

    pn532_packetbuffer[0] = HMS_PN532_COMMAND_READREGISTER;
    pn532_packetbuffer[1] = (registerAddress >> 8) & 0xFF;
    pn532_packetbuffer[2] = registerAddress & 0xFF;

    if (interface->write(pn532_packetbuffer, 3) != HMS_PN532_OK) {
        return 0;
    }

    // read data packet
    if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer)) != HMS_PN532_OK) {
        return 0;
    }

    response = pn532_packetbuffer[0];

    return response;
}

uint32_t HMS_PN532_Controller::writeRegister(uint16_t registerAddress, uint8_t value) {
    uint32_t response;

    pn532_packetbuffer[0] = HMS_PN532_COMMAND_WRITEREGISTER;
    pn532_packetbuffer[1] = (registerAddress >> 8) & 0xFF;
    pn532_packetbuffer[2] = registerAddress & 0xFF;
    pn532_packetbuffer[3] = value;


    if (interface->write(pn532_packetbuffer, 4) != HMS_PN532_OK) {
        return 0;
    }

    // read data packet
    if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer)) != HMS_PN532_OK) {
        return 0;
    }

    response = pn532_packetbuffer[0];

    return response;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::setRFField(uint8_t autoRFCA, uint8_t rFOnOff) {
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_RFCONFIGURATION;
  pn532_packetbuffer[1] = 1;
  pn532_packetbuffer[2] = 0x00 | autoRFCA | rFOnOff;  

  if (interface->write(pn532_packetbuffer, 3) != HMS_PN532_OK) {
      return HMS_PN532_ERROR;
  }

  return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::setPassiveActivationRetries(uint8_t maxRetries) {
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_RFCONFIGURATION;
  pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
  pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
  pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
  pn532_packetbuffer[4] = maxRetries;

  if (interface->write(pn532_packetbuffer, 5) != HMS_PN532_OK)
    return HMS_PN532_ERROR;

  return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::inListPassiveTarget() {
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;
  pn532_packetbuffer[2] = 0;

  #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.debug("inList passive target");
  #endif

  if (interface->write(pn532_packetbuffer, 3) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  };

  if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer), 30000) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  if (pn532_packetbuffer[0] != 1) {
    return HMS_PN532_ERROR;
  }

  inListedTag = pn532_packetbuffer[1];

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::inDataExchange(uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength) {
  uint8_t i;

  pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = inListedTag;

  if (interface->write(pn532_packetbuffer, 2) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  if (interface->read(response, *responseLength, 1000) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  if ((response[0] & 0x3f) != 0) {
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("inDataExchange: status is not ok");
    #endif
    return HMS_PN532_ERROR;
  }

  for (uint8_t i = 0; i < *responseLength; i++) {
    response[i] = response[i + 1];
  }

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::inRelease(const uint8_t relevantTarget){

    pn532_packetbuffer[0] = HMS_PN532_COMMAND_INRELEASE;
    pn532_packetbuffer[1] = relevantTarget;

    if (interface->write(pn532_packetbuffer, 2) != HMS_PN532_OK) {
        return HMS_PN532_ERROR;
    }

    // read data packet
    return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::tgInitAsTarget(uint16_t timeout) {
  const uint8_t command[] = {
    HMS_PN532_COMMAND_TGINITASTARGET,
    0,
    0x00, 0x00,         //SENS_RES
    0x00, 0x00, 0x00,   //NFCID1
    0x40,               //SEL_RES

    0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, // POL_RES
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF,

    0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, 0x00, 0x00, //NFCID3t: Change this to desired value

    0x06, 0x46,  0x66, 0x6D, 0x01, 0x01, 0x10, 0x00// LLCP magic number and version parameter
  };
  return tgInitAsTarget(command, sizeof(command), timeout);
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::tgInitAsTarget(const uint8_t* command, const uint8_t len, const uint16_t timeout) {
  if (interface->write(command, len) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout);
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::tgGetData(uint8_t *buf, uint8_t len) {
  buf[0] = HMS_PN532_COMMAND_TGGETDATA;

  if (interface->write(buf, 1) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  if (interface->read(buf, len, 3000) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  if (buf[0] != 0) {
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("tgGetData: status is not ok");
    #endif
    return HMS_PN532_ERROR;
  }

  memmove(buf, buf + 1, len - 1);

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::tgSetData(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen) {
  if (hlen > (sizeof(pn532_packetbuffer) - 1)) {
    if ((body != 0) || (header == pn532_packetbuffer)) {
        #if HMS_PN532_DEBUG_ENABLED
          pn532Logger.error("tgSetData:buffer too small");
        #endif
        return HMS_PN532_ERROR;
      }

    pn532_packetbuffer[0] = HMS_PN532_COMMAND_TGSETDATA;
    if (interface->write(pn532_packetbuffer, 1) != HMS_PN532_OK) {
      return HMS_PN532_ERROR;
    }
  } else {
    for (int8_t i = hlen - 1; i >= 0; i--){
      pn532_packetbuffer[i + 1] = header[i];
    }
    pn532_packetbuffer[0] = HMS_PN532_COMMAND_TGSETDATA;

    if (interface->write(pn532_packetbuffer, hlen + 1) != HMS_PN532_OK) {
      return HMS_PN532_ERROR;
    }
  }

  if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer), 3000) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  if (0 != pn532_packetbuffer[0]) {
    return HMS_PN532_ERROR;
  }

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaRelease() {
    pn532_packetbuffer[0] = HMS_PN532_COMMAND_INRELEASE;
    pn532_packetbuffer[1] = 0x00;                                                                   // All target

    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.debug("Release all FeliCa target");
    #endif

    if(interface->write(pn532_packetbuffer, 2) != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("No ACK");
        #endif
        return HMS_PN532_INVALID_ACK;  // no ACK
    }

  // Wait card response
    if(interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000) != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
                pn532Logger.error("Could not receive response");
        #endif
        return HMS_PN532_TIMEOUT;
    }

  // Check status (pn532_packetbuffer[0])
  if ((pn532_packetbuffer[0] & 0x3F)!=0) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Status code indicates an error: %02X", pn532_packetbuffer[7]);
    #endif
    return HMS_PN532_INVALID_FRAME;
  }

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaRequestResponse(uint8_t * mode) {
  uint8_t cmd[9];
  cmd[0] = HMS_PN532_FELICA_CMD_REQUEST_RESPONSE;
  memcpy(&cmd[1], felicaIDm, 8);

  uint8_t response[10];
  uint8_t responseLength;
  if (felicaSendCommand(cmd, 9, response, &responseLength) != HMS_PN532_OK) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Request Response command failed");
    #endif
    return HMS_PN532_ERROR;
  }

  // length check
  if ( responseLength != 10) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Request Response command failed (wrong response length)");
    #endif
    return HMS_PN532_ERROR;
  }

  *mode = response[9];
  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaRequestSystemCode(uint8_t * numSystemCode, uint16_t *systemCodeList) {
  uint8_t cmd[9];
  cmd[0] = HMS_PN532_FELICA_CMD_REQUEST_SYSTEM_CODE;
  memcpy(&cmd[1], felicaIDm, 8);

  uint8_t response[10 + 2 * 16];
  uint8_t responseLength;
  if (felicaSendCommand(cmd, 9, response, &responseLength) != HMS_PN532_OK) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Request System Code command failed");
    #endif
    return HMS_PN532_ERROR;
  }
  *numSystemCode = response[9];

  // length check
  if ( responseLength < 10 + 2 * *numSystemCode ) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Request System Code command failed (wrong response length)");
    #endif
    return HMS_PN532_ERROR;
  }

  uint8_t i;
  for(i=0; i<*numSystemCode; i++) {
    systemCodeList[i] = (uint16_t)((response[10+i*2]<< 8) + response[10+i*2+1]);
  }

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaRequestService(uint8_t numNode, uint16_t *nodeCodeList, uint16_t *keyVersions) {
  if (numNode > HMS_PN532_FELICA_REQ_SERVICE_MAX_NODE_NUM) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("numNode is too large");
    #endif
    return HMS_PN532_ERROR;
  }

  uint8_t i, j=0;
  uint8_t cmdLen = 1 + 8 + 1 + 2*numNode;
  uint8_t cmd[cmdLen];
  cmd[j++] = HMS_PN532_FELICA_CMD_REQUEST_SERVICE;
  for (i=0; i<8; ++i) {
    cmd[j++] = felicaIDm[i];
  }
  cmd[j++] = numNode;
  for (i=0; i<numNode; ++i) {
    cmd[j++] = nodeCodeList[i] & 0xFF;
    cmd[j++] = (nodeCodeList[i] >> 8) & 0xff;
  }

  uint8_t response[10+2*numNode];
  uint8_t responseLength;

  if (felicaSendCommand(cmd, cmdLen, response, &responseLength) != HMS_PN532_OK) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Request Service command failed");
    #endif
    return HMS_PN532_ERROR;
  }

  // length check
  if ( responseLength != 10+2*numNode ) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Request Service command failed (wrong response length)");
    #endif
    return HMS_PN532_ERROR;
  }

  for(i=0; i<numNode; i++) {
    keyVersions[i] = (uint16_t)(response[10+i*2] + (response[10+i*2+1] << 8));
  }
  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaSendCommand (const uint8_t *command, uint8_t commandlength, uint8_t *response, uint8_t *responseLength) {
    if (commandlength > 0xFE) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Command length too long");
        #endif
    return HMS_PN532_INVALID_COMMAND;
    }

    pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = inListedTag;
    pn532_packetbuffer[2] = commandlength + 1;

    if (interface->write(pn532_packetbuffer, 3, command, commandlength) != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Could not send FeliCa command");
        #endif
        return HMS_PN532_INVALID_ACK;
    }

    // Wait card response
    if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer), 200) != HMS_PN532_OK) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("Could not receive response");
        #endif
        return HMS_PN532_INVALID_FRAME;
    }

  // Check status (pn532_packetbuffer[0])
  if ((pn532_packetbuffer[0] & 0x3F)!=0) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Status code indicates an error: %02X", pn532_packetbuffer[0]);
    #endif
    return HMS_PN532_ERROR;
  }

  *responseLength = pn532_packetbuffer[1] - 1;

  memcpy(response, &pn532_packetbuffer[2], *responseLength);

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaPolling(uint16_t systemCode, uint8_t requestCode, uint8_t * idm, uint8_t * pmm, uint16_t *systemCodeResponse, uint16_t timeout) {
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;
  pn532_packetbuffer[2] = 1;
  pn532_packetbuffer[3] = HMS_PN532_FELICA_CMD_POLLING;
  pn532_packetbuffer[4] = (systemCode >> 8) & 0xFF;
  pn532_packetbuffer[5] = systemCode & 0xFF;
  pn532_packetbuffer[6] = requestCode;
  pn532_packetbuffer[7] = 0;

  if (interface->write(pn532_packetbuffer, 8) != HMS_PN532_OK) {
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("Could not send Polling command");  
    #endif
    return HMS_PN532_INVALID_ACK;
  }

  if (interface->read(pn532_packetbuffer, 22, timeout) != HMS_PN532_OK) {
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("Could not receive response");
    #endif
    return HMS_PN532_INVALID_ACK;
  }

  // Check NbTg (pn532_packetbuffer[7])
  if (pn532_packetbuffer[0] == 0) {
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("No card had detected");
    #endif
    return HMS_PN532_NOT_FOUND;
  } else if (pn532_packetbuffer[0] != 1) {
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("Unhandled number of targets inlisted. NbTg: %02X", pn532_packetbuffer[7]);
    #endif
    return HMS_PN532_ERROR;
  }

  inListedTag = pn532_packetbuffer[1];

  #if HMS_PN532_DEBUG_ENABLED
    pn532Logger.info("Tag number: %02X", inListedTag);
  #endif

  // length check
  uint8_t responseLength = pn532_packetbuffer[2];
  if (responseLength != 18 && responseLength != 20) {
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("Wrong response length: %02X", responseLength);
    #endif
    return HMS_PN532_ERROR;
  }

  uint8_t i;
  for (i=0; i<8; ++i) {
    idm[i] = pn532_packetbuffer[4+i];
    felicaIDm[i] = pn532_packetbuffer[4+i];
    pmm[i] = pn532_packetbuffer[12+i];
    felicaPMm[i] = pn532_packetbuffer[12+i];
  }

  if ( responseLength == 20 ) {
    *systemCodeResponse = (uint16_t)((pn532_packetbuffer[20] << 8) + pn532_packetbuffer[21]);
  }

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaReadWithoutEncryption(uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16]) {
    if (numService > HMS_PN532_FELICA_READ_MAX_SERVICE_NUM) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("numService is too large");
        #endif
        return HMS_PN532_NO_SPACE;
    }
    if (numBlock > HMS_PN532_FELICA_READ_MAX_BLOCK_NUM) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("numBlock is too large");
        #endif
        return HMS_PN532_NO_SPACE;
    }

    uint8_t i, j=0, k;
    uint8_t cmdLen = 1 + 8 + 1 + 2*numService + 1 + 2*numBlock;
    uint8_t cmd[cmdLen];
    cmd[j++] = HMS_PN532_FELICA_CMD_READ_WITHOUT_ENCRYPTION;
    for (i=0; i<8; ++i) {
        cmd[j++] = felicaIDm[i];
    }
    cmd[j++] = numService;
    for (i=0; i<numService; ++i) {
        cmd[j++] = serviceCodeList[i] & 0xFF;
        cmd[j++] = (serviceCodeList[i] >> 8) & 0xff;
    }
    cmd[j++] = numBlock;
    for (i=0; i<numBlock; ++i) {
        cmd[j++] = (blockList[i] >> 8) & 0xFF;
        cmd[j++] = blockList[i] & 0xff;
    }

  uint8_t response[12+16*numBlock];
  uint8_t responseLength;
  if (felicaSendCommand(cmd, cmdLen, response, &responseLength) != HMS_PN532_OK) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Read Without Encryption command failed");
    #endif
    return HMS_PN532_ERROR;
  }

  // length check
  if ( responseLength != 12+16*numBlock ) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Read Without Encryption command failed (wrong response length)");
    #endif
    return HMS_PN532_ERROR;
  }

  // status flag check
  if ( response[9] != 0 || response[10] != 0 ) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Read Without Encryption command failed (Status Flag: %02X %02X)", response[9], response[10]);
    #endif
    return HMS_PN532_ERROR;
  }

  k = 12;
  for(i=0; i<numBlock; i++ ) {
    for(j=0; j<16; j++ ) {
      blockData[i][j] = response[k++];
    }
  }

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::felicaWriteWithoutEncryption(uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16]) {
    if (numService > HMS_PN532_FELICA_WRITE_MAX_SERVICE_NUM) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("numService is too large");
        #endif
        return HMS_PN532_ERROR;
    }

    if (numBlock > HMS_PN532_FELICA_WRITE_MAX_BLOCK_NUM) {
        #if HMS_PN532_DEBUG_ENABLED
            pn532Logger.error("numBlock is too large");
        #endif
        return HMS_PN532_NO_SPACE;
    }

  uint8_t i, j=0, k;
  uint8_t cmdLen = 1 + 8 + 1 + 2*numService + 1 + 2*numBlock + 16 * numBlock;
  uint8_t cmd[cmdLen];
  cmd[j++] = HMS_PN532_FELICA_CMD_WRITE_WITHOUT_ENCRYPTION;
  for (i=0; i<8; ++i) {
    cmd[j++] = felicaIDm[i];
  }
  cmd[j++] = numService;
  for (i=0; i<numService; ++i) {
    cmd[j++] = serviceCodeList[i] & 0xFF;
    cmd[j++] = (serviceCodeList[i] >> 8) & 0xff;
  }
  cmd[j++] = numBlock;
  for (i=0; i<numBlock; ++i) {
    cmd[j++] = (blockList[i] >> 8) & 0xFF;
    cmd[j++] = blockList[i] & 0xff;
  }
  for (i=0; i<numBlock; ++i) {
    for(k=0; k<16; k++) {
      cmd[j++] = blockData[i][k];
    }
  }

  uint8_t response[11];
  uint8_t responseLength;
  if (felicaSendCommand(cmd, cmdLen, response, &responseLength) != HMS_PN532_OK) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Write Without Encryption command failed");
    #endif
    return HMS_PN532_ERROR;
  }

  // length check
  if ( responseLength != 11 ) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Write Without Encryption command failed (wrong response length)");
    #endif
    return HMS_PN532_ERROR;
  }

  // status flag check
  if ( response[9] != 0 || response[10] != 0 ) {
    #if HMS_PN532_DEBUG_ENABLED
        pn532Logger.error("Write Without Encryption command failed (Status Flag: %02X %02X)", response[9], response[10]);
    #endif
    return HMS_PN532_ERROR;
  }

  return HMS_PN532_OK;
}


HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareclassicIsFirstBlock (uint32_t uiBlock) {
  // Test if we are in the small or big sectors
  if (uiBlock < 128)
    return ((uiBlock) % 4 == 0) ? HMS_PN532_OK : HMS_PN532_ERROR;
  else
    return ((uiBlock) % 16 == 0) ? HMS_PN532_OK : HMS_PN532_ERROR;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareclassicIsTrailerBlock (uint32_t uiBlock) {
  // Test if we are in the small or big sectors
  if (uiBlock < 128)
    return ((uiBlock + 1) % 4 == 0) ? HMS_PN532_OK : HMS_PN532_ERROR;
  else
    return ((uiBlock + 1) % 16 == 0) ? HMS_PN532_OK : HMS_PN532_ERROR;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareclassicReadDataBlock (uint8_t blockNumber, uint8_t *data) {
  #if HMS_PN532_DEBUG_ENABLED
    pn532Logger.debug("Trying to read 16 bytes from block %d", blockNumber);
  #endif

  /* Prepare the command */
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = HMS_PN532_MAX_CARD_NUM_SCAN;                                                                                            // Card number
  pn532_packetbuffer[2] = HMS_PN532_MIFARE_CMD_READ;                                                                                              // Mifare Read command = 0x30
  pn532_packetbuffer[3] = blockNumber;                                                                                                            // Block Number (0..63 for 1K, 0..255 for 4K)

  if (interface->write(pn532_packetbuffer, 4) != HMS_PN532_OK) {                                                                                  // Send the command
    return HMS_PN532_ERROR;
  }

  interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));                                                                                // Read the response packet

  if (pn532_packetbuffer[0] != 0x00) {
    return HMS_PN532_ERROR;
  }

  memcpy (data, pn532_packetbuffer + 1, 16);
  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareclassicAuthenticateBlock (uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData) {
  uint8_t index;

  memcpy (this->key, keyData, 6);                                                                                                               // Cache the key and uid data
  memcpy (this->uid, uid, uidLen);
  this->uidLen = uidLen;

  /* Prepare the authentication command */
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_INDATAEXCHANGE;                                                                                     // Data Exchange Header
  pn532_packetbuffer[1] = HMS_PN532_MAX_CARD_NUM_SCAN;                                                                                          // Max card numbers
  pn532_packetbuffer[2] = (keyNumber) ? HMS_PN532_MIFARE_CMD_AUTH_B : HMS_PN532_MIFARE_CMD_AUTH_A;
  pn532_packetbuffer[3] = blockNumber;                                                                                                          // Block Number (1K = 0..63, 4K = 0..255
 
  memcpy (pn532_packetbuffer + 4, this->key, 6);
  for (index = 0; index < this->uidLen; index++) {
    pn532_packetbuffer[10 + index] = this->uid[index];                                                                                          // 4 bytes card ID
  }

  if (interface->write(pn532_packetbuffer, 10 + this->uidLen) != HMS_PN532_OK)  return HMS_PN532_ERROR;

  interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));                                                                              // Read the response packet

  if (pn532_packetbuffer[0] != 0x00) {                                                                                                          // Success would be bytes 5-7: 0xD5 0x41 0x00 (Mifare auth error is technically byte 7: 0x14)
    #if HMS_PN532_DEBUG_ENABLED
      pn532Logger.error("Authentification failed: %02X", pn532_packetbuffer[0]);
    #endif
    return HMS_PN532_ERROR;
  }

  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid, uint8_t &uidLength, uint16_t timeout) {
  pn532_packetbuffer[2] = cardbaudrate;
  pn532_packetbuffer[1] = HMS_PN532_MAX_CARD_NUM_SCAN;
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_INLISTPASSIVETARGET;
  
  if (interface->write(pn532_packetbuffer, 3) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;                                                                                                                         // command failed
  }

  if (interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout) != HMS_PN532_OK) {                                                   // read data packet
    return HMS_PN532_ERROR;
  }

    /*
      ┌─────────────────────────────────────────────────────────────────────┐
      │ Note: ISO14443A card response should be in the following format     │
      │                                                                     │
      │  Byte         │ Description                                         │
      │ --------------│------------------------------------------           │
      │  b0           │ Tags Found                                          │
      │  b1           │ Tag Number (only one used in this example)          │
      │  b2..3        │ SENS_RES                                            │
      │  b4           │ SEL_RES                                             │
      │  b5           │ NFCID Length                                        │
      │  b6..NFCIDLen │ NFCID                                               │
      └─────────────────────────────────────────────────────────────────────┘
    */

  if (pn532_packetbuffer[0] != HMS_PN532_MAX_CARD_NUM_SCAN)
    return HMS_PN532_NOT_FOUND;

  uint16_t sens_res = pn532_packetbuffer[2];
  sens_res <<= 8;
  sens_res |= pn532_packetbuffer[3];

  #if HMS_PN532_DEBUG_ENABLED
    pn532Logger.debug("ATQA: 0x%04X", sens_res);
    pn532Logger.debug("SAK: 0x%02X", pn532_packetbuffer[4]);
    pn532Logger.debug("UID Length: %d", pn532_packetbuffer[5]);
  #endif

  uidLength = pn532_packetbuffer[5];

  for (uint8_t i = 0; i < pn532_packetbuffer[5]; i++) {
    uid[i] = pn532_packetbuffer[6 + i];
  }

  return HMS_PN532_OK;
}


HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareclassicWriteDataBlock (uint8_t blockNumber, uint8_t *data) {
  /* Prepare the first command */
  pn532_packetbuffer[0] = HMS_PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = 1;                      /* Card number */
  pn532_packetbuffer[2] = HMS_PN532_MIFARE_CMD_WRITE;       /* Mifare Write command = 0xA0 */
  pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
  memcpy (pn532_packetbuffer + 4, data, 16);        /* Data Payload */

  /* Send the command */
  if (interface->write(pn532_packetbuffer, 20) != HMS_PN532_OK) {
    return HMS_PN532_ERROR;
  }

  /* Read the response packet */
  return interface->read(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareclassicFormatNDEF (void) {
  uint8_t sectorbuffer1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
  uint8_t sectorbuffer2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
  uint8_t sectorbuffer3[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  // Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
  // for the MAD sector in NDEF records (sector 0)

  // Write block 1 and 2 to the card
  if (mifareclassicWriteDataBlock (1, sectorbuffer1) != HMS_PN532_OK)
      return HMS_PN532_ERROR;
  if (mifareclassicWriteDataBlock (2, sectorbuffer2) != HMS_PN532_OK)
      return HMS_PN532_ERROR;
  // Write key A and access rights card
  if (mifareclassicWriteDataBlock (3, sectorbuffer3) != HMS_PN532_OK)
      return HMS_PN532_ERROR;

  // Seems that everything was OK (?!)
  return HMS_PN532_OK;
}

HMS_PN532_StatusTypeDef HMS_PN532_Controller::mifareclassicWriteNDEFURI (uint8_t sectorNumber, uint8_t uriIdentifier, const char *url) {
    // Figure out how long the string is
    uint8_t len = strlen(url);

    // Make sure we're within a 1K limit for the sector number
    if ((sectorNumber < 1) || (sectorNumber > 15))
        return HMS_PN532_ERROR;

    // Make sure the URI payload is between 1 and 38 chars
    if ((len < 1) || (len > 38))
        return HMS_PN532_ERROR;

    // Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
    // in NDEF records

    // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
    uint8_t sectorbuffer1[16] = {0x00, 0x00, 0x03, uint8_t(len + 5), 0xD1, 0x01, uint8_t(len + 1), 0x55, uriIdentifier, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (len <= 6) {
        // Unlikely we'll get a url this short, but why not ...
        memcpy (sectorbuffer1 + 9, url, len);
        sectorbuffer1[len + 9] = 0xFE;
    } else if (len == 7) {
        // 0xFE needs to be wrapped around to next block
        memcpy (sectorbuffer1 + 9, url, len);
        sectorbuffer2[0] = 0xFE;
    } else if ((len > 7) && (len <= 22)) {
        // Url fits in two blocks
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, len - 7);
        sectorbuffer2[len - 7] = 0xFE;
    } else if (len == 23) {
        // 0xFE needs to be wrapped around to final block
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, len - 7);
        sectorbuffer3[0] = 0xFE;
    } else {
        // Url fits in three blocks
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, 16);
        memcpy (sectorbuffer3, url + 23, len - 23);
        sectorbuffer3[len - 23] = 0xFE;
    }

    // Now write all three blocks back to the card
    if (mifareclassicWriteDataBlock (sectorNumber * 4, sectorbuffer1) != HMS_PN532_OK)
        return HMS_PN532_ERROR;
    if (mifareclassicWriteDataBlock ((sectorNumber * 4) + 1, sectorbuffer2) != HMS_PN532_OK)
        return HMS_PN532_ERROR;
    if (mifareclassicWriteDataBlock ((sectorNumber * 4) + 2, sectorbuffer3) != HMS_PN532_OK)
        return HMS_PN532_ERROR;
    if (mifareclassicWriteDataBlock ((sectorNumber * 4) + 3, sectorbuffer4) != HMS_PN532_OK)
        return HMS_PN532_ERROR;

    // Seems that everything was OK (?!)
    return HMS_PN532_OK;
}