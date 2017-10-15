#include <nds.h>
#include <stdio.h>
#include <stdarg.h>

#include "platform.h"
#include "console.h"
#include "ntrcard.h"
#include "delay.h"
#include "nds_platform.h"

using namespace std;
using namespace flashcart_core;

void _sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp, uint32_t flags) {
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
}

bool platform::sendCommand(const std::uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp, ntrcard::OpFlags flags) {
    _sendCommand(cmdbuf, response_len, resp, flags.pack());
    return true;
}

void platform::showProgress(uint32_t curr, uint32_t total, const char *status_string) {
    printProgress(status_string, curr, total);
}

void platform::ioDelay(unsigned long delay) {
    ioDelay(delay);
}

int32_t platform::resetCard() {
    reset();
    return 0;
}

void platform::initBlowfishPS(std::uint32_t (&ps)[ntrcard::BLOWFISH_PS_N]) {
}

#ifdef DEBUG_PRINT
int Flashcart::logMessage(log_priority priority, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    iprintf("\n");
    viprintf(fmt, args);
    va_end(args);

    return 0;
}
#endif

const uint8_t dummyCommand[8] = {0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t chipIDCommand[8] = {0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void reset() {
    uint8_t *garbage = (uint8_t*)malloc(sizeof(uint8_t) * 0x2000);
    if (!garbage) {
        // FIXME
        return;
    }
    _sendCommand(dummyCommand, 0x2000, garbage, 32);
    free(garbage);
}

uint32_t getChipID() {
    uint32_t chipID;
    reset();
    _sendCommand(chipIDCommand, 4, (uint8_t*)&chipID, 32);
    return chipID;
}
