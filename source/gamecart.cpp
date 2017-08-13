#include <nds.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "gamecart/protocol_ntr.h"

void Cart_NTRInit() {
}

void NTR_SendCommand(const u8 *command, u32 length, u32 unk, u8 *destination) {
    u8 reversed[8];
    for (int i = 0; i < 8; i++) {
        reversed[7 - i] = command[i];
    }

    u32 defaultFlags;
    switch (length & 0xfffffffc) {
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

    /*
    switch(command[0]) {
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
    */

    // NDSL only
    cardPolledTransfer(defaultFlags | CARD_ACTIVATE | CARD_nRESET,
                       (u32*)destination, length, reversed);
    // NDSL, DSLi, etc...
    //cardPolledTransfer(flags | defaultFlags | CARD_ACTIVATE | CARD_nRESET |
    //                       CARD_SEC_CMD | CARD_SEC_EN | CARD_SEC_DAT,
    //                   destination, length, command);
    return;
}
