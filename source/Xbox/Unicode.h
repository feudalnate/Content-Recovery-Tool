#ifndef XBOXUNICODE
#define XBOXUNICODE

#include <xtl.h>
#include "Kernel.h"

BOOL UnicodeToString(PWCHAR unicode_string, PCHAR &string, DWORD &string_length);
BOOL StringToUnicode(PCHAR string, PWCHAR &unicode_string, DWORD &unicode_string_length);

#endif