#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ak2i.h"

struct ntrcardhax_info {
    int32_t version;
    u32 ntrcard_header_addr;
    u32 rtfs_cfg_addr;
    u32 rtfs_handle_addr;
};

#define AK2I_PATCH_LENGTH 0x20000
#define AK2I_PAYLOAD_OFFSET 0x2000
#define AK2I_PAYLOAD_LENGTH 0x1000

static u8 bootrom[AK2I_PATCH_LENGTH];
static u8 payload[AK2I_PAYLOAD_LENGTH];
static int payloadLength = 0x1000;

static uint16_t crc16tab[] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static struct ntrcardhax_info o_ak2i_ntrcardhax_infos[6] = {
    { 17120, 0x80e18f4, 0x80e4c0c, 0x80ec190 },
    { 18182, 0x80e18f4, 0x80e4c0c, 0x80ec190 },
    { 19216, 0x80e18f4, 0x80e4c0c, 0x80ec190 },
    { 20262, 0x80e18f4, 0x80e4c0c, 0x80ec190 },
    { 21288, 0x80e1cb4, 0x80e4bcc, 0x80ec150 },
    { 22313, 0x80e1cb4, 0x80e4bcc, 0x80ec150 },
};

static struct ntrcardhax_info n_ak2i_ntrcardhax_infos[6] = {
    { 17120, 0x80e2b34, 0x80e5e4c, 0x80ed3d0 },
    { 18182, 0x80e1974, 0x80e4c8c, 0x80ec210 },
    { 19218, 0x80e1974, 0x80e4c8c, 0x80ec210 },
    { 20262, 0x80e1974, 0x80e4c8c, 0x80ec210 },
    { 21288, 0x80f9d34, 0x80fcc4c, 0x81041d0 },
    { 22313, 0x80f9d34, 0x80fcc4c, 0x81041d0 },
};

//	 ldr sp,=0x22140000
//
//	 ;Disable IRQ
//	 mrs r0, cpsr
//	 orr r0, #(1<<7)
//	 msr cpsr_c, r0
//
//	 adr r0, kernelmode
//	 swi 0x7B
//
//kernelmode:
//	 mov r2, #0x22
//	 msr CPSR_c, #0xDF
//	 ldr r3, =0x33333333 ;R/W
//	 mcr p15, 0, r3,c5,c0, 2
//	 mov r2, #0xCC
//	 mcr p15, 0, r3,c5,c0, 3
//	 ldr r0, =0x23F00000
//	 bx r0
static u8 loader_bin[0x44] =
{
    0x30, 0xD0, 0x9F, 0xE5, 0x00, 0x00, 0x0F, 0xE1, 0x80, 0x00, 0x80, 0xE3, 0x00, 0xF0, 0x21, 0xE1,
    0x00, 0x00, 0x8F, 0xE2, 0x7B, 0x00, 0x00, 0xEF, 0x22, 0x20, 0xA0, 0xE3, 0xDF, 0xF0, 0x21, 0xE3,
    0x14, 0x30, 0x9F, 0xE5, 0x50, 0x3F, 0x05, 0xEE, 0xCC, 0x20, 0xA0, 0xE3, 0x70, 0x3F, 0x05, 0xEE,
    0x08, 0x00, 0x9F, 0xE5, 0x10, 0xFF, 0x2F, 0xE1, 0x00, 0x00, 0x14, 0x22, 0x33, 0x33, 0x33, 0x33,
    0x00, 0x00, 0xF0, 0x23,
};


void pause() {
	iprintf("press <A>\n");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_A)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

void waitCartReady() {
    u32 chipid;
    u32 hwid;

    do {
        iprintf("Please eject & reinsert cartridge.\n");
        pause();

        chipid = getChipID();
        hwid = getHWID();

        iprintf("CHIP: 0x%04X HW: %X\n", chipid, hwid);
    } while (chipid != 0xFC2 || (hwid != 0x81 && hwid != 0x44));
}

int percent(int c, int t) {
    return c * 100 / t;
}

uint16_t calcCrc(u8 *data, uint32_t length)
{
    uint16_t crc = (uint16_t)~0;
    for(unsigned int i = 0; i<length; i++)
    {
        crc = (crc >> 8) ^ crc16tab[(crc ^ data[i]) & 0xFF];
    }
    return crc;
}

int dump() {
    // recheck;
    u32 chipid = getChipID();
    if (chipid != 0xFC2) {
        iprintf("Cartridge is not AK2i\n");
        return -1;
    }
    u32 hwid = getHWID();
    if (hwid != 0x81 && hwid != 0x44) {
        iprintf("Cartridge is not AK2i\n");
        return -1;
    }

    setMapTableAddress(AK2I_MTN_NOR_OFFSET, 0);

    // setup flash to 1681
    if (hwid == 0x81) {
        iprintf("Setup flash to 1681\n");
        setFlash1681_81();
    }

    iprintf("Active FAT MAP\n");
    activeFatMap();

    int old = 0;

    iprintf("0%%");
    for (u32 i = 0, offset = 0, read = 0; i < AK2I_PATCH_LENGTH; i += 512) {
        int curr = percent(i, AK2I_PATCH_LENGTH);
        if (curr != old && curr % 5 == 0) {
            iprintf("\r%d%%", curr);
            old = curr;
        }
        u32 toRead = 512;
        if (toRead > AK2I_PATCH_LENGTH - i) {
            toRead = AK2I_PATCH_LENGTH - i;
        }
        memset(bootrom + offset, 0, toRead);
        readFlash(i, bootrom + offset, toRead);
        offset += toRead;
    }
    iprintf("\r100%%\n");

    iprintf("Done\n\n");

//    for (u32 i = 0; i < AK2I_PATCH_LENGTH; i += 16) {
//        printf("%05X:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
//               i,
//               bootrom[i], bootrom[i + 1], bootrom[i + 2], bootrom[i + 3],
//               bootrom[i + 4], bootrom[i + 5], bootrom[i + 6], bootrom[i + 7],
//               bootrom[i + 8], bootrom[i + 9], bootrom[i + 10], bootrom[i + 11],
//               bootrom[i + 12], bootrom[i + 13], bootrom[i + 14], bootrom[i + 15]);
//
//        if (i % 10 == 0) {
//            pause();
//        }
//    }
//    pause();

    return 0;
}

int8_t selectDeviceType() {
    iprintf("Use arrow keys\nand <A> to choose device type\n");

    u8 is_new = 0;
    while (true) {
        iprintf("\r%s", is_new ? "NEW 3DS" : "OLD 3DS");

        scanKeys();
        u32 keys = keysDown();
        if (keys & (KEY_UP | KEY_DOWN)) {
            is_new = (is_new + 1) & 1;
        } else if (keys & KEY_A) {
            iprintf("\n\n");
            return is_new;
        } else if (keys & KEY_B) {
            iprintf("\r(cancelled by user)\n\n");
            return -1;
        }
        swiWaitForVBlank();
    }
}

int32_t selectFirmVersion(int8_t device) {
    iprintf("Use arrow keys\nand <A> to choose target firm version\n");

    const u32 o_ver[6] = {17120, 18182, 19216, 20262, 21288, 22313};
    const u32 n_ver[6] = {17120, 18182, 19218, 20262, 21288, 22313};

    const char* versions[6] = {
        "(9.0 - 9.2)  ",
        "(9.3 - 9.4)  ",
        "(9.5)        ",
        "(9.6 - 9.9)  ",
        "(10.0 - 10.1)",
        "(10.2 - 10.3)"
    };

    int idx = 0;
    while (true) {
        iprintf("\rv%d %s", (device ? n_ver[idx] : o_ver[idx]), versions[idx]);

        scanKeys();
        u32 keys = keysDown();
        if (keys & KEY_UP) {
            idx -= 1;
            if (idx < 0) {
                idx = 0;
            }
        } else if (keys & KEY_DOWN) {
            idx += 1;
            if (idx > 5) {
                idx = 5;
            }
        } else if (keys & KEY_A) {
            iprintf("\n\n");
            return device ? n_ver[idx] : o_ver[idx];
        } else if (keys & KEY_B) {
            iprintf("\r(cancelled by user)\n\n");
            return -1;
        }

        swiWaitForVBlank();
    }
}

int patch() {
    int8_t device = selectDeviceType();
    if (device < 0) {
        return -1;
    }

    int32_t version = selectFirmVersion(device);
    if (version < 0) {
        return -1;
    }

    struct ntrcardhax_info *info;
    for (int i = 0; i < 6; i++) {
        struct ntrcardhax_info *tmp = device
                                    ? &n_ak2i_ntrcardhax_infos[i]
                                    : &o_ak2i_ntrcardhax_infos[i];
        if (tmp->version == version) {
            info = tmp;
            break;
        }
    }

    memcpy(payload, bootrom + AK2I_PAYLOAD_OFFSET, AK2I_PAYLOAD_LENGTH);

    // see https://github.com/peteratebs/rtfsprofatfilesystem/blob/b0003c4/include/rtfstypes.h#L1468-L1549
    int rtfsCfgAdrDiff = info->rtfs_cfg_addr - info->ntrcard_header_addr;
    // use ~ drno_to_dr_map
    int rtfsCopyLen = 0x144;

    int wrappedAdr = (rtfsCfgAdrDiff) & 0xFFF;

    if ((wrappedAdr >= 0x0) && (wrappedAdr <= 0x10)) {
        iprintf("Conflict ntrcard header");
        return -1;
    }
    if ((wrappedAdr >= 0x2A8) && (wrappedAdr <= 0x314)) {
        iprintf("Conflict rtfs struct");
        return -1;
    }

    uint32_t rtfs_cfg[0x144] = {0};
    // cfg_NFINODES
    rtfs_cfg[5] = 1;
    // mem_region_pool
    rtfs_cfg[17] = (uint32_t)(info->ntrcard_header_addr + 0x4);
    int i;
    // drno_to_dr_map
    for (i = 0; i < 26; i++)
        rtfs_cfg[55 + i] = (uint32_t)(info->ntrcard_header_addr + 0);

    uint32_t* prtfs_cfg32 = rtfs_cfg;

    for (i = 0; i < rtfsCopyLen; i+=4) {
        wrappedAdr = (rtfsCfgAdrDiff + i) & 0xFFF;
        //printf("addr: %08X data: %08X\n", wrappedAdr, prtfs_cfg32[i/4]);
        if((wrappedAdr >= 0x14) && (wrappedAdr <= 0x60)) {
            //if(i < 0xFC) {
            //    iprintf("Not enough buffer");
            //    return -1;
            //}
            break;
        }
        *(uint32_t*)&payload[wrappedAdr] = prtfs_cfg32[i/4];
    }

    *(uint32_t*)&payload[0x2EC] = info->rtfs_handle_addr; //Some handle rtfs uses
    *(uint32_t*)&payload[0x2F0] = 0x41414141; //Bypass FAT corruption error
    *(uint32_t*)&payload[0x31C] = info->ntrcard_header_addr + 0x2A8; //This is the PC we want to jump to (from a BLX)

    memcpy(&payload[0x2A8], loader_bin, 0x44);

    uint16_t crc = calcCrc(payload, 0x15E);
    *(uint16_t*)&payload[0x15E] = crc;

    memcpy(bootrom + AK2I_PAYLOAD_OFFSET, payload, AK2I_PAYLOAD_LENGTH);

    iprintf("crc: %x\n", crc);

    return 0;
}


int inject() {
    u32 chipid = getChipID();
    if (chipid != 0xFC2) {
        iprintf("Cartridge is not AK2i\n");
        return -1;
    }

    u32 hwid = getHWID();
    if (hwid != 0x81 && hwid != 0x44 ) {
        iprintf("Cartridge is not AK2i\n");
        return -1;
    }

    iprintf("Cart: 0x%04X HW: %x\n", chipid, hwid);

    setMapTableAddress(AK2I_MTN_NOR_OFFSET, 0);

    // setup flash to 1681
    if (hwid == 0x81) {
        iprintf("Setup flash to 1681\n");
        setFlash1681_81();
    }

    iprintf("Active FAT MAP\n");
    // have to do this on ak2i before making fatmap, or the first 128k flash data will be screwed up.
    activeFatMap();

    iprintf("Unlock flash\n");
    // this funcion write enable only above 0x40000 on ak2i, write enable all on ak2
    unlockFlash();

    iprintf("Unlock ASIC");
    // unlock 0x30000 for save map, see definition of NOR_FAT2_START above
    unlockASIC();

    iprintf("Erase flash");
    for (u32 i = 0; i < AK2I_PATCH_LENGTH; i += 64 * 1024) {
        if (hwid == 0x81) {
            eraseFlashBlock_81(i);
        } else {
            eraseFlashBlock_44(i);
        }
    }
    //ioAK2EraseFlash( 0, CHIP_ERASE );

    //buffer[0x200C] = 'B';

    iprintf("Writing...");

    int old = 0;

    iprintf("%0");
    for (u32 i = 0; i < AK2I_PATCH_LENGTH; i += 512) {
        int curr = percent(i, AK2I_PATCH_LENGTH);
        if (curr != old && curr % 5 == 0) {
            iprintf("\r%d%%", curr);
            old = curr;
        }
        if (hwid == 0x81) {
            writeFlash_81(i, bootrom + i, 512);
        } else {
            writeFlash_44(i, bootrom + i, 512);
        }

        if (!verifyFlash(bootrom + i, i, 512)) {
            iprintf("verify failed at %08X\n", i);
        }
    }
    iprintf("\r100%%\n");

    iprintf("Lock flash\n");
    lockFlash();

    iprintf("Done\n\n");

    return 0;
}


void dumpAndPatch() {
    if (dump() < 0) {
        iprintf("Failed\n\n");
        return;
    }

    if (patch() < 0) {
        iprintf("Failed\n\n");
        return;
    }

    if (inject() < 0) {
        iprintf("Failed\n\n");
        return;
    }
}

int main(void) {

    consoleDemoInit();

    sysSetBusOwners(true, true); // give ARM9 access to the cart

    iprintf("AK2I ntrcardhax injector\n");
    iprintf("* application will remove *\n");
    iprintf("* nds flashcart feature   *\n");
    iprintf("* if you want recover,    *\n");
    iprintf("* need another flashcart  *\n");

    pause();

    waitCartReady();
    dumpAndPatch();

    pause();
    return 0;
}
