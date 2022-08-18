#ifndef HEXADECIMAL
#define HEXADECIMAL

#include <xtl.h>

BOOL ValidHexString(CHAR* string, DWORD length);
VOID CopyHexToBytes(CHAR* string, DWORD length, BYTE* buffer, BOOL reverse);

#endif