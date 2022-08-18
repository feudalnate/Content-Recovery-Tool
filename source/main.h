#include <xtl.h>
#include <stdio.h>
#include <string.h>

#include <vector>
using namespace std; //normally avoid std's in my code but I _really_ don't want to do a 2D linked list, forgive me

#include "Xbox\Kernel.h"
#include "Xbox\Hexadecimal.h"
#include "Xbox\Unicode.h"
#include "Xbox\Directory.h"
#include "Xbox\UI\Console.h"

#define XBEBaseAddress	0x00010000
#define XBEMagic		0x58424548
#define XCSFMagic		0x46534358

const PWCHAR FontFileW = L"D:\\font.ttf";

const CHAR PartitionLetter = 'N'; //temp. mount letter
const PCHAR PartitionPath = "\\Device\\Harddisk0\\Partition1"; // E:

const PCHAR TitleDataRegion = "T:\\"; //symlink of TitleId specific TDATA folder
const PCHAR UserDataRegion =  "U:\\"; //symlink of TitleId specific UDATA folder

struct SearchResult {
	DWORD TitleId;
	DWORD FileCount;
	vector<PCHAR>* Files;
};

BOOL SearchContentMetaFiles(DWORD &TotalFiles, vector<SearchResult*>* &Results);
VOID FreeSearchResults(vector<SearchResult*>* &Results);

BOOL SignContentMeta(PCHAR File, DWORD TitleId, BOOL OnlySignIfInvalid, BOOL &FailedToAccessFile, BOOL &BadFileSize, BOOL &BadHeaderSize, BOOL &WasInvalidMagic, BOOL &WasSkippedBecauseValid, BOOL &WasSimpleContentMeta);
BOOL CalculateContentHeaderSignature(DWORD TitleId, PBYTE buffer, DWORD length, PBYTE result);

DWORD GetAlternateTitleId();
VOID SetAlternateTitleId(DWORD AlternateTitleId);

BOOL CheckXBEHeader();
BOOL MemoryPageWritable(LPVOID Address);

BOOL MountPartition(PCHAR PartitionPath, CHAR PartitionLetter);
BOOL UnmountPartition(CHAR PartitionLetter);

DWORD AwaitButtonPress();