#pragma once

void waitPressA();
void enablePrintProgress();
void disablePrintProgress();
void printProgress(const char *string, uint32_t curr, uint32_t total);
