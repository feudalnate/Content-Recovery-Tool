#ifndef XBOXDIRHELPER
#define XBOXDIRHELPER

#include <xtl.h>
#include "Unicode.h"

BOOL FileExists(PCHAR Path);
BOOL FileExists(PWCHAR Path);

BOOL DirectoryExists(PCHAR Path);
BOOL DirectoryExists(PWCHAR Path);

#endif