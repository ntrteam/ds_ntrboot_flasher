#include <nds.h>
#include <stdio.h>
#include <stdarg.h>

#include "platform.h"
#include "console.h"
#include "ntrcard.h"
#include "delay.h"
#include "nds_platform.h"

extern uint8_t *blowfish_ntr_bin;
extern uint8_t *blowfish_retail_bin;
extern uint8_t *blowfish_dev_bin;

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

extern const bool platform::HAS_HW_KEY2 = true;

bool platform::sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp, ntrcard::OpFlags flags) {
    _sendCommand(cmdbuf, response_len, resp, flags);
    return true;
}

void platform::showProgress(uint32_t curr, uint32_t total, const char *status_string) {
    printProgress(status_string, curr, total);
}

void platform::ioDelay(unsigned long delay) {
    swiDelay(delay);
}

int32_t platform::resetCard() {
    reset();
    return 0;
}

void platform::initBlowfishPS(uint32_t (&ps)[ntrcard::BLOWFISH_PS_N], ntrcard::BlowfishKey key) {
    if (sizeof(ps) != 0x1048) {
        return;
    }

    const void *ptr;
    switch (key) {
        default: // blah
        case ntrcard::BlowfishKey::NTR:
            ptr = blowfish_ntr_bin;
            break;
        case ntrcard::BlowfishKey::B9RETAIL:
            ptr = blowfish_retail_bin;
            break;
        case ntrcard::BlowfishKey::B9DEV:
            ptr = blowfish_dev_bin;
            break;
    }

    memcpy(ps, ptr, sizeof(ps));
}

void platform::initKey2Seed(std::uint64_t x, std::uint64_t y) {
    REG_ROMCTRL = 0;

    uint32_t xl = (uint32_t)(x & 0xFFFFFFFF);
    uint32_t yl = (uint32_t)(y & 0xFFFFFFFF);
    uint16_t xh = (uint16_t)((x >> 32) & 0x7F);
    uint16_t yh = (uint16_t)((y >> 32) & 0x7F);

    REG_CARD_1B0 = xl;
    REG_CARD_1B4 = yl;
    REG_CARD_1B8 = xh;
    REG_CARD_1BA = yh;

    REG_ROMCTRL = CARD_nRESET| CARD_SEC_SEED | CARD_SEC_EN | CARD_SEC_DAT;
}

#ifdef DEBUG_PRINT
int platform::logMessage(log_priority priority, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    iprintf("\n");
    viprintf(fmt, args);
    va_end(args);

    return 0;
}
#endif

void reset() {
    uint8_t *garbage = (uint8_t*)malloc(sizeof(uint8_t) * 0x2000);
    if (!garbage) {
        // FIXME
        return;
    }
    cardReadHeader(garbage);
    free(garbage);
}

uint32_t getChipID() {
    reset();
    return cardReadID(32);
}
