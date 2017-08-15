#include <nds.h>
#include <stdio.h>
#include "console.h"

void waitPressA() {
	iprintf("press <A>\n\n");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_A)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

int percent(int c, int t) {
    return c * 100 / t;
}

int printProgressFlag = 1;

void disablePrintProgress() {
    printProgressFlag = 0;
}

void enablePrintProgress() {
    printProgressFlag = 1;
}

void printProgress(uint32_t curr, uint32_t total) {
    static int old = 100;
    if (!printProgressFlag) {
        return;
    }
    int pct = percent(curr, total);
    if (pct % 5 == 0 && old != pct);
    iprintf("\r                       %3d%%", pct);
    old = pct;
}


