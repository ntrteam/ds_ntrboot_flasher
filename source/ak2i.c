#include <stdio.h>
#include <unistd.h>
#include "ak2i.h"

void ak2iCardWriteAndRead(const u8 *command, u32 length, u32 flags,
                          u32 *destination) {
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
                       destination, length, command);
    // NDSL, DSLi, etc...
    //cardPolledTransfer(flags | defaultFlags | CARD_ACTIVATE | CARD_nRESET |
    //                       CARD_SEC_CMD | CARD_SEC_EN | CARD_SEC_DAT,
    //                   destination, length, command);
    return;
}

u32 getChipID() {
    const u8 command[8] = {0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, CARD_CMD_DATA_CHIPID};
    u32 ret = 0;
    ak2iCardWriteAndRead(command, 4, 0, &ret);
    return ret;
}

u16 getHWID() {
    const u8 command[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD1};
    u32 ret = 0;
    ak2iCardWriteAndRead(command, 4, 0, &ret);
    return (u16)ret & 0xFF;
}

void setMapTableAddress(u32 tableName, u32 tableInRamAddress) {
    tableName &= 0x0F;
    u32 cmd[2] = {0xD0000000 | (tableInRamAddress >> 8),
        ((tableInRamAddress & 0xFF) << 24) | ((u8)tableName << 16) };
    const u8 command[8] = {
        cmd[1] >>  0 & 0xFF,
        cmd[1] >>  8 & 0xFF,
        cmd[1] >> 16 & 0xFF,
        cmd[1] >> 24 & 0xFF,
        cmd[0] >>  0 & 0xFF,
        cmd[0] >>  8 & 0xFF,
        cmd[0] >> 16 & 0xFF,
        cmd[0] >> 24 & 0xFF,
    };
    ak2iCardWriteAndRead(command, 0, 0, NULL);
}

void setFlash1681_81() {
    const u8 command[8] = {0x06, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8};
    ak2iCardWriteAndRead(command, 0, CARD_DELAY1(20), NULL);
}

void activeFatMap() {
    const u8 command[8] = {0x00, 0x00, 0x00, 0xAA, 0x55, 0xAA, 0x55, 0xC2};
    ak2iCardWriteAndRead(command, 4, 0, NULL);
}

void unlockFlash() {
    const u8 command[8] = {0x00, 0x00, 0x00, 0x55, 0xAA, 0x55, 0xAA, 0xC2};
    ak2iCardWriteAndRead(command, 0, 0, NULL);
}

void unlockASIC() {
    const u8 command[8] = {0x00, 0x00, 0x00, 0xAA, 0x55, 0x55, 0xAA, 0xC2};
    ak2iCardWriteAndRead(command, 4, 0, NULL);
}

void lockFlash(void) {
    const u8 command[8] = {0x00, 0x00, 0x00, 0x55, 0x55, 0xAA, 0xAA, 0xC2};
    ak2iCardWriteAndRead(command, 0, 0, NULL);
}

void readFlash(u32 address, u8 *buffer, u32 length) {
    length &= ~(0x03);
    u32 cmd[2] = { 0xB7000000 | (address >> 8), (address & 0xff) << 24 | 0x00100000 };

    const u8 command[8] = {
        cmd[1] >>  0 & 0xFF,
        cmd[1] >>  8 & 0xFF,
        cmd[1] >> 16 & 0xFF,
        cmd[1] >> 24 & 0xFF,
        cmd[0] >>  0 & 0xFF,
        cmd[0] >>  8 & 0xFF,
        cmd[0] >> 16 & 0xFF,
        cmd[0] >> 24 & 0xFF,
    };
    ak2iCardWriteAndRead(command, length, CARD_DELAY1(2), buffer);
}

static void waitFlashBusy() {
    u32 state = 0;
    const u8 command[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0};
    do {
        //ioAK2Delay( 16 * 10 );
        ak2iCardWriteAndRead(command, 4, CARD_DELAY1(4), &state);
        state &= 1;
    } while(state != 0);
}

void eraseFlashBlock_44(u32 address) {
    u32 cmd[2] = {0xD4000000 | (address & 0x001fffff), (u32)(1<<16)};

    const u8 command[8] = {
        cmd[1] >>  0 & 0xFF,
        cmd[1] >>  8 & 0xFF,
        cmd[1] >> 16 & 0xFF,
        cmd[1] >> 24 & 0xFF,
        cmd[0] >>  0 & 0xFF,
        cmd[0] >>  8 & 0xFF,
        cmd[0] >> 16 & 0xFF,
        cmd[0] >> 24 & 0xFF,
    };
    ak2iCardWriteAndRead(command, 0, 0, NULL);
    waitFlashBusy();
}

void eraseFlashBlock_81(u32 address) {
    u32 cmd[2] = {0xD4000000 | (address & 0x001fffff), (u32)((0x30<<24) | (0x80<<16) | (0<<8) | (0x35))};

    const u8 command[8] = {
        cmd[1] >>  0 & 0xFF,
        cmd[1] >>  8 & 0xFF,
        cmd[1] >> 16 & 0xFF,
        cmd[1] >> 24 & 0xFF,
        cmd[0] >>  0 & 0xFF,
        cmd[0] >>  8 & 0xFF,
        cmd[0] >> 16 & 0xFF,
        cmd[0] >> 24 & 0xFF,
    };
    ak2iCardWriteAndRead(command, 0, CARD_DELAY1(20), NULL);
    waitFlashBusy();
}

void writeFlashByte_44(u32 address, u8 data) {
    u32 cmd[2] = {0xD4000000 | (address & 0x001fffff), (u32)((data<<24) | (3<<16))};
    const u8 command[8] = {
        cmd[1] >>  0 & 0xFF,
        cmd[1] >>  8 & 0xFF,
        cmd[1] >> 16 & 0xFF,
        cmd[1] >> 24 & 0xFF,
        cmd[0] >>  0 & 0xFF,
        cmd[0] >>  8 & 0xFF,
        cmd[0] >> 16 & 0xFF,
        cmd[0] >> 24 & 0xFF,
    };
    ak2iCardWriteAndRead(command, 0, CARD_DELAY1(20), NULL);
    waitFlashBusy();
}

void writeFlash_44(u32 address, const void *data, u32 length) {
    u8 * pbuffer = (u8 *)data;
    for(u32 i = 0; i < length; ++i) {
        writeFlashByte_44(address, *(pbuffer + i));
        address++;
    }
}

void writeFlashByte_81(u32 address, u8 data) {
    u32 cmd[2] = { 0xD4000000 | (address & 0x001fffff), (u32)((data<<24) | (0xa0<<16) | (0<<8) | (0x63)) };
    const u8 command[8] = {
        cmd[1] >>  0 & 0xFF,
        cmd[1] >>  8 & 0xFF,
        cmd[1] >> 16 & 0xFF,
        cmd[1] >> 24 & 0xFF,
        cmd[0] >>  0 & 0xFF,
        cmd[0] >>  8 & 0xFF,
        cmd[0] >> 16 & 0xFF,
        cmd[0] >> 24 & 0xFF,
    };
    ak2iCardWriteAndRead(command, 0, CARD_DELAY1(20), NULL);
    waitFlashBusy();
}

void writeFlash_81(u32 address, const void *data, u32 length) {
    u8 * pbuffer = (u8 *)data;
    for (u32 i = 0; i < length; ++i) {
        writeFlashByte_81(address, *(pbuffer + i));
        address++;
    }
}

bool verifyFlash(void *src, u32 dest, u32 length) {
    u8 verifyBuffer[512];
    u8 * pSrc = (u8 *)src;
    for (u32 i = 0; i < length; i += 512) {
        u32 toRead = 512;
        if (toRead > length - i)
            toRead = length - i;
        readFlash(dest + i, verifyBuffer, toRead);

        for (u32 j = 0; j < toRead; ++j) {
            if(verifyBuffer[j] != *(pSrc + i + j))
                return false;
        }
    }
    return true;
}

void command_d0() {
    const u8 command[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0};
    ak2iCardWriteAndRead(command, 0, 0, NULL);
}

u32 command_c2(u8 cmd1, u8 cmd2, u32 length) {
    u32 ret = 0;
    const u8 command[8] = {0x00, 0x00, 0x00, cmd2, cmd1, 0x55, 0xaa, 0xc2};
    ak2iCardWriteAndRead(command, length, 0, &ret);
    return ret;
}

void command_c8() {
    const u8 command[8] = {0x06, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc8};
    ak2iCardWriteAndRead(command, 0, CARD_DELAY1(20), NULL);
}

void command_d4(u32 addr, u8 data, u32 cmd) {
    u8 command[8] = {cmd & 0xff, cmd >> 8 & 0xff, cmd >> 16 & 0xff, data,
                     addr & 0xff, addr >> 8 & 0xff, addr >> 16 & 0xff, 0xd4};
    ak2iCardWriteAndRead(command, 0, CARD_DELAY1(20), NULL);
}

void command_c0() {
    u32 ret = 0;
    u8 command[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0};
    do {
        ak2iCardWriteAndRead(command, 4, 0, &ret);
    } while (ret & 1);
}

u32 getProgress(u32 cur, u32 start, u32 end) {
    return (cur - start) * 100 / (end - start);
}

void flash_8181() {
    // start update
    command_c2(0xaa, 0x55, 0);
    command_c2(0x55, 0xaa, 4);
    command_c8();

    // 0x2000324

    for (u32 addr = 0x80000; addr < 0x1ce000; addr += 0x10000) {
        command_d4(addr, 0x30, 0x800035);
        command_c0();
    }

    // write
    // TODO read from firm file
    u8* newFirmPos = (u8*)0x2009ecc;
    u32 prog = 0;
    iprintf("0%%");
    for (u32 addr = 0x80000; addr < 0x1ce000; addr += 1, newFirmPos += 1) {
        u32 cur = getProgress(addr, 0x80000, 0x1ce000);
        if (cur != prog && (cur % 5) == 0) {
            iprintf("..%lu%%", cur);
            prog = cur;
        }
        command_d4(addr, *newFirmPos, 0xa000e3);
        command_c0();
    }
    iprintf("\nDone\n");
}

void flash_4444() {
    // start update
    command_c2(0xaa, 0x55, 0);
    command_c8();

    // 0x2000324

    for (u32 addr = 0x80000; addr < 0x1ce000; addr += 0x10000) {
        command_d4(addr, 1, 0);
        command_c0();
    }

    // write
    // TODO read from firm file
    u8* newFirmPos = (u8*)0x2009ecc;
    u32 prog = 0;
    iprintf("0%%");
    for (u32 addr = 0x80000; addr < 0x1ce000; addr += 1, newFirmPos += 1) {
        u32 cur = getProgress(addr, 0x80000, 0x1ce000);
        if (cur != prog && (cur % 5) == 0) {
            iprintf("..%lu%%", cur);
            prog = cur;
        }
        command_d4(addr, *newFirmPos, 0x830000);
        command_c0();
    }
    iprintf("\nDone\n");
}

void flash(u16 hwid) {
    command_d0();
    iprintf("Updating, DO NOT turn off DS.\n");
    switch (hwid) {
        case 0x8181:
            flash_8181();
            return;
        case 0x4444:
            flash_4444();
            return;
    }
}

void readFirm(u32 *dest) {
}

void dump(u16 hwid) {
    char *fn;

    iprintf("Start dump, DO NOT turn off DS.\n");
    iprintf("0%%");
    u32 offset = 0;
    u32 data;
    u32 prog = 0;
    do {
        u8 command[8] = {0, 0, 0, 0,
                         offset & 0xff, offset >> 8 & 0xff, offset >> 16 & 0xff,
                         CARD_CMD_HEADER_READ};
        command[6] = offset >> 16 & 0xff;
        command[5] = offset >> 8 & 0xff;
        command[4] = offset & 0xff;
        ak2iCardWriteAndRead(command, 4, 0, &data);
        offset += 4;
        u32 cur = getProgress(offset, 0, 0x20000);
        if (cur != prog && (cur % 5) == 0) {
            prog = cur;
        }
        iprintf("%lx", data);
    } while (offset != 0x20000);
    printf("\n");
    iprintf("\nDone\n");
}
