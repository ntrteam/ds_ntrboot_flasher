#include <nds.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "gamecart/protocol_ntr.h"

void Cart_NTRInit() {
}

void NTR_SendCommand(const u8 *command, u32 length, u32 flags, u8 *destination) {
    cardWriteCommand(command);
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
    // NDSL only
    cardPolledTransfer(flags | defaultFlags | CARD_ACTIVATE | CARD_nRESET,
                       (u32*)destination, length, command);
    // NDSL, DSLi, etc...
    //cardPolledTransfer(flags | defaultFlags | CARD_ACTIVATE | CARD_nRESET |
    //                       CARD_SEC_CMD | CARD_SEC_EN | CARD_SEC_DAT,
    //                   destination, length, command);
    return;
}
