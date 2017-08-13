#ifndef PROTOCOL_NTR_H
#define PROTOCOL_NTR_H
void Cart_NTRInit();
void NTR_SendCommand(const uint8_t *command, uint32_t length, uint32_t flags, uint8_t *destination);
#endif
