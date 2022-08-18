#include "Directory.h"

BOOL FileExists(PCHAR Path) {

	if (!Path)
		return FALSE;

	DWORD Attributes = GetFileAttributes(Path);

	return (Attributes != -1);
}

BOOL FileExists(PWCHAR Path) {

	if (!Path)
		return FALSE;

	PCHAR PathA = (PCHAR)malloc(MAX_PATH);
	ZeroMemory(PathA, MAX_PATH);

	DWORD PathALen = 0;

	if (!UnicodeToString(Path, PathA, PathALen)) {

		free(PathA);
		return FALSE;
	}

	BOOL Result = FileExists(PathA);
	free(PathA);

	return Result;
}

BOOL DirectoryExists(PCHAR Path) {

	if (!Path)
		return FALSE;
	
	DWORD Attributes = GetFileAttributes(Path);

	return ((Attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
}

BOOL DirectoryExists(PWCHAR Path) {

	if (!Path)
		return FALSE;

	PCHAR PathA = (PCHAR)malloc(MAX_PATH);
	ZeroMemory(PathA, MAX_PATH);

	DWORD PathALen = 0;

	if (!UnicodeToString(Path, PathA, PathALen)) {

		free(PathA);
		return FALSE;
	}

	BOOL Result = DirectoryExists(PathA);
	free(PathA);

	return Result;
}






