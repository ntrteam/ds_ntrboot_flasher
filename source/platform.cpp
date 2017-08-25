#include <nds.h>
#include <stdio.h>

#include "flashcart_core/device.h"
#include "console.h"

void Flashcart::platformInit() {
    // TODO
}

void Flashcart::sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp, uint32_t flags) {
    u8 reversed[8];
    for (int i = 0; i < 8; i++) {
        reversed[7 - i] = cmdbuf[i];
    }

    u32 defaultFlags = flags;
    switch (response_len & 0xfffffffc) {
        case 0:
            defaultFlags |= 0;
            break;
        case 4:
            defaultFlags |= CARD_BLK_SIZE(7);
            break;
        case 512:
            defaultFlags |= CARD_BLK_SIZE(1);
            break;
        case 8192:
            defaultFlags |= CARD_BLK_SIZE(5);
            break;
        case 16384:
            defaultFlags |= CARD_BLK_SIZE(6);
            break;
        default:
            defaultFlags |= CARD_BLK_SIZE(4);
            break;
    }

#ifndef NDSI_MODE
    // NDSL only
    cardPolledTransfer(defaultFlags | CARD_ACTIVATE | CARD_nRESET,
                       (u32*)resp, response_len, reversed);
#else
    // NDSL, DSLi, etc...
    cardPolledTransfer(defaultFlags | CARD_ACTIVATE | CARD_nRESET |
                       CARD_SEC_CMD | CARD_SEC_EN | CARD_SEC_DAT,
                       (u32*)resp, response_len, reversed);
#endif
    return;
}

void Flashcart::showProgress(uint32_t curr, uint32_t total) {
    printProgress(curr, total);
}
