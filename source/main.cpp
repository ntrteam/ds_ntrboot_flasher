#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <flashcart_core/device.h>

#include "console.h"
#include "binaries.h"

// FIXME: not fully overwrite
u8 orig_flashrom[0xA0000] = {0};
u8 curr_flashrom[0xA0000] = {0};
u8 restorable = 0;

Flashcart* waitCartReady() {
    Flashcart *cart = NULL;
    do {
        iprintf("Please eject & reinsert cartridge.\n");
        iprintf("If you use non AK2i cart,\n");
        iprintf("remove SDCARD from your cartridge.\n");
        waitPressA();

        iprintf("ChipID: %08X\n", Flashcart::getChipID());
        iprintf("HW Rev: %08X\n", Flashcart::getHardwareVersion());

        cart = Flashcart::detectCart();
    } while(!cart);

    iprintf("Detected: %s\n\n", cart->getDescription());

    return cart;
}

u8 dump(Flashcart *cart) {
    u32 length = cart->getMaxLength();
    if (length > 0xA0000) {
        length = 0xA0000;
    }
    iprintf("%X\n", length);
    memset(orig_flashrom, 0, 0xA0000);
    u8 *temp = orig_flashrom;

    iprintf("Dump original flashrom\n");
    printProgress(0, 1);
    cart->readFlash(0, length, temp);
    cart->cleanup();
    printProgress(1, 1);

    restorable = 1;
    iprintf("\nDone\n\n");

    /*
    for (int i = 0; i < length; i+=16) {
        iprintf("%05X:", i);
        for (int j = 0; j < 16; j++) {
            iprintf("%02X ", orig_flashrom[i + j]);
        }
        iprintf("\n");
        waitPressA();
    }
    */

    return 0;
}

int8_t selectDeviceType() {
    iprintf("Use arrow keys\nand <A> to choose device type\n");

    u8 is_dev = 0;
    while (true) {
        iprintf("\r%s", is_dev ? "DEV   " : "RETAIL");

        scanKeys();
        u32 keys = keysDown();
        if (keys & (KEY_UP | KEY_DOWN)) {
            is_dev = (is_dev + 1) & 1;
        } else if (keys & KEY_A) {
            iprintf("\n\n");
            return is_dev;
        } else if (keys & KEY_B) {
            iprintf("\r(cancelled by user)\n\n");
            return -1;
        }
        swiWaitForVBlank();
    }
}

int load() {
    if (restorable) {
        iprintf("already loaded original rom\n");
        iprintf("you will lost this data\n");
        iprintf("and you can lost restore chance\n");
        iprintf("Y. continue\n");
        iprintf("A. cancel\n");

        while (true) {
            scanKeys();
            u32 keys = keysDown();

            if (keys & KEY_A) {
                return -1;
            } else if (keys & KEY_Y) {
                break;
            }
            swiWaitForVBlank();
        }
    }

    Flashcart *cart = waitCartReady();
    return dump(cart);
}

int inject() {
    if (!restorable) {
        iprintf("you haven't load firmware\n");
        iprintf("it can make to lost flashcart\n");
        iprintf("feature\n");
        iprintf("please load first\n");
        waitPressA();
        return -1;
    }

    Flashcart *cart = waitCartReady();

    u8 deviceType = selectDeviceType();
    if (deviceType < 0) {
        return -1;
    }

    u8 *blowfish_key = deviceType ? blowfish_dev_bin : blowfish_retail_bin;
    u8 *firm = deviceType ? boot9strap_ntr_dev_firm : boot9strap_ntr_firm;
    u32 firm_size = deviceType ? boot9strap_ntr_dev_firm_size : boot9strap_ntr_firm_size;

    iprintf("Flash ntrboothax\n");
    printProgress(0, 1);
    cart->writeBlowfishAndFirm(blowfish_key, firm, firm_size);
    cart->cleanup();
    printProgress(1, 1);
    iprintf("\nDone\n\n");

    return 0;
}

int compareBuf(u8 *buf1, u8 *buf2, u32 len) {
    for (int i = 0; i < len; i++) {
        if (buf1[i] != buf2[i]) {
            return 0;
        }
    }
    return 1;
}

int restore() {
    Flashcart *cart = waitCartReady();

    if (!restorable) {
        iprintf("Not loaded original flashrom\n");
        return -1;
    }

    u32 length = cart->getMaxLength();
    if (length > 0xA0000) {
        length = 0xA0000;
    }

    memset(curr_flashrom, 0, 0xA0000);
    u8 *temp = curr_flashrom;

    iprintf("Read current flashrom\n");
    printProgress(0, 1);
    cart->readFlash(0, length, temp);
    printProgress(1, 1);

    iprintf("\nRestore original flash\n");

    const int chunk_size = 64 * 1024;
    printProgress(0, 1);
    for (int i = 0; i < length; i += chunk_size) {
        printProgress(i, length);
        disablePrintProgress();
        if (!compareBuf(orig_flashrom + i, curr_flashrom + i, chunk_size)) {
            cart->writeFlash(i, chunk_size, orig_flashrom + i);
        }
        enablePrintProgress();
    }
    printProgress(1, 1);

    cart->cleanup();

    memset(curr_flashrom, 0, 0xA0000);
    temp = curr_flashrom;

    iprintf("\nReload current flashrom\n");
    printProgress(0, 1);
    cart->readFlash(0, length, temp);
    printProgress(1, 1);

    iprintf("\nVerify...\n");

    printProgress(0, 1);
    for (int i = 0; i < length; i += chunk_size) {
        printProgress(i, length);
        if (!compareBuf(orig_flashrom + i, curr_flashrom + i, chunk_size)) {
            iprintf("\nfail");
            goto exit;
        }
    }
    printProgress(1, 1);
    iprintf("\nok");

exit:
    cart->cleanup();
    iprintf("\nDone\n\n");


    // TODO verify check
    return 0;
}

int main(void) {

    consoleDemoInit();

    sysSetBusOwners(true, true); // give ARM9 access to the cart

    iprintf("= AK2I NTRBOOTHAX FLASHER =\n\n");

    iprintf("* if you use non ak2i     *\n");
    iprintf("* you will lost flashcart *\n");
    iprintf("* feature.                *\n");
    iprintf("* DO NOT CLOSE THIS APP   *\n");
    iprintf("* IF YOU DONT HAVE SAME   *\n");
    iprintf("* FLASHCART               *\n");

    waitPressA();

    while (true) {
        iprintf("You can swap flashcart for flash\n");
        iprintf("X. load flashrom\n");
        iprintf("A. inject ntrboothax\n");
        iprintf("Y. restore flashrom\n");
        iprintf("B. exit\n\n");

        while (true) {
            scanKeys();
            u32 keys = keysDown();

            if (keys & KEY_A) {
                inject();
                break;
            } else if (keys & KEY_X) {
                load();
                break;
            } else if (keys & KEY_Y) {
                restore();
                break;
            } else if (keys & KEY_B) {
                return 0;
            }
            swiWaitForVBlank();
        }
    }

    return 0;
}
