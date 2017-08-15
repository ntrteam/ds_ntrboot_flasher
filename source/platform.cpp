#include <nds.h>
#include <stdio.h>

#include "flashcart_core/device.h"
#include "console.h"

void Flashcart::platformInit() {
    // TODO
}

void Flashcart::sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp) {
    u8 reversed[8];
    for (int i = 0; i < 8; i++) {
        reversed[7 - i] = cmdbuf[i];
    }

    u32 defaultFlags;
    switch (response_len & 0xfffffffc) {
        case 0:
            defaultFlags = 0;
            break;
        case 4:
            defaultFlags = CARD_BLK_SIZE(7);
            break;
        case 512:
            defaultFlags = CARD_BLK_SIZE(1);
            break;
        case 8192:
            defaultFlags = CARD_BLK_SIZE(5);
            break;
        case 16384:
            defaultFlags = CARD_BLK_SIZE(6);
            break;
        default:
            defaultFlags = CARD_BLK_SIZE(4);
            break;
    }

    switch (cmdbuf[0]) {
        case 0xA5:
            defaultFlags |= CARD_DELAY1(20);
            break;
        case 0xB7:
            defaultFlags |= CARD_DELAY1(2);
            break;
        case 0xC0:
            defaultFlags |= CARD_DELAY1(4);
            break;
        case 0xC8:
        case 0xD4:
        case 0xD8:
            defaultFlags |= CARD_DELAY1(20);
            break;
    }

    // NDSL only
    cardPolledTransfer(defaultFlags | CARD_ACTIVATE | CARD_nRESET,
                       (u32*)resp, response_len, reversed);
    // NDSL, DSLi, etc...
    //cardPolledTransfer(flags | defaultFlags | CARD_ACTIVATE | CARD_nRESET |
    //                       CARD_SEC_CMD | CARD_SEC_EN | CARD_SEC_DAT,
    //                   destination, length, command);
    return;
}

void Flashcart::showProgress(uint32_t curr, uint32_t total) {
    printProgress(curr, total);
}
