#ifndef AK2I_H
#define AK2I_H

#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>

enum {
    AK2I_MTN_NOR_OFFSET      = 0,
};

u32 getChipID();
u16 getHWID();

void setMapTableAddress(u32 tableName, u32 tableInRamAddress);
void setFlash1681_81();
void activeFatMap();
void unlockFlash();
void unlockASIC();
void lockFlash();

void readFlash(u32 address, u8 *buffer, u32 length);

void eraseFlashBlock_44(u32 address);
void eraseFlashBlock_81(u32 address);

void writeFlash_44(u32 address, const void *data, u32 length);
void writeFlash_81(u32 address, const void *data, u32 length);
bool verifyFlash(void *src, u32 dest, u32 length);

#ifdef __cplusplus
}
#endif
#endif
