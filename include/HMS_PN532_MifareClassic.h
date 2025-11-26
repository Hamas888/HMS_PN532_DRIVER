#ifndef HMS_PN532_MIFARECLASSIC_H
#define HMS_PN532_MIFARECLASSIC_H

#include "HMS_PN532_Config.h"
#include "HMS_PN532_NFC_Tag.h"
#include "HMS_PN532_Controller.h"

#define MIFARECLASSIC_TYPE_NAME                               "Mifare Classic"

#define MIFARECLASSIC_BLOCK_SIZE                              16
#define MIFARECLASSIC_LONG_TLV_SIZE                           4
#define MIFARECLASSIC_SHORT_TLV_SIZE                          2

#define MIFARECLASSIC_NR_SHORTSECTOR                          32                                                      // Number of short sectors on Mifare 1K/4K
#define MIFARECLASSIC_NR_LONGSECTOR                           8                                                       // Number of long sectors on Mifare 4K
#define MIFARECLASSIC_NR_BLOCK_OF_SHORTSECTOR                 4                                                       // Number of blocks in a short sector
#define MIFARECLASSIC_NR_BLOCK_OF_LONGSECTOR                  16                                                      // Number of blocks in a long sector

#define MIFARECLASSIC_BLOCK_NUMBER_OF_SECTOR_TRAILER(sector)  ( \
  (                                                             \
    (sector) < MIFARECLASSIC_NR_SHORTSECTOR                     \
  ) ? (                                                         \
    (sector) * MIFARECLASSIC_NR_BLOCK_OF_SHORTSECTOR +          \
    MIFARECLASSIC_NR_BLOCK_OF_SHORTSECTOR - 1                   \
  ) : (                                                         \
    MIFARECLASSIC_NR_SHORTSECTOR *                              \
    MIFARECLASSIC_NR_BLOCK_OF_SHORTSECTOR +                     \
    (sector - MIFARECLASSIC_NR_SHORTSECTOR) *                   \
    MIFARECLASSIC_NR_BLOCK_OF_LONGSECTOR +                      \
    MIFARECLASSIC_NR_BLOCK_OF_LONGSECTOR - 1                    \
  )                                                             \
)                                                                                                                   // Determine the sector trailer block based on sector number


class HMS_PN532_MifareClassic {
    public:
        HMS_PN532_MifareClassic(HMS_PN532_Controller& controller);
        ~HMS_PN532_MifareClassic();

        HMS_PN532_NFC_Tag readTag(byte *uid, uint8_t uidLength);
        HMS_PN532_StatusTypeDef formatNDEF(byte *uid, uint8_t uidLength);
        HMS_PN532_StatusTypeDef formatMifare(byte *uid, uint8_t uidLength);
        HMS_PN532_StatusTypeDef writeTag(HMS_PN532_NDEF_Message &ndefMessage, byte *uid, uint8_t uidLength);
    private:
        HMS_PN532_Controller *controller;

        int getNdefStartIndex(byte *data);
        int getBufferSize(int messageLength);
        bool decodeTLV(byte *data, int &messageLength, int &messageStartIndex);
};

#endif // HMS_PN532_MIFARECLASSIC_H