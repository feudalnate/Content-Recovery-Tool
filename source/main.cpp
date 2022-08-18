#include "main.h"

XDEVICE_PREALLOC_TYPE Controllers[] = { { XDEVICE_TYPE_GAMEPAD, 4 } };

void main() {

	if (FileExists(FontFileW))
		ConsoleInit(640, 480, FontFileW, 14);
	else
		ConsoleInit(640, 480, NULL, NULL);

	XInitDevices((sizeof(Controllers) / sizeof(XDEVICE_PREALLOC_TYPE)), Controllers);

	while(!ConsoleIsInitialized())
		Sleep(20);

	ConsoleWriteLine(2);
	ConsoleWriteLine(L"     Content Recovery Tool (1.0)");
	ConsoleWriteLine(L"     https://github.com/feudalnate");
	ConsoleWriteLine(L"     ____________________________________________________________________");
	ConsoleWriteLine(1);
	ConsoleWriteLine(L"     Re-signs downloadable content and updates to the current XboxHDKey");
	ConsoleWriteLine(L"     Restores \"corrupted\" content after an intentional (or unintentional) XboxHDKey change");
	ConsoleWriteLine(L"     ____________________________________________________________________");
	ConsoleWriteLine(1);
	ConsoleWriteLine(L"     A: Search for content packages");
	ConsoleWriteLine(L"     B: Exit to dashboard");
	ConsoleWriteLine(2);

	while(1) {

		switch(AwaitButtonPress()) {

			case XINPUT_GAMEPAD_A:
				goto SearchContent;

			case XINPUT_GAMEPAD_B:
				goto Exit;

			default:
				continue;

		}

	}

SearchContent:

	ConsoleWriteLine(L"     Depending on the amount of contents on the hard drive, this could take a minute");
	ConsoleWriteLine(L"     Searching for content packages...");
	ConsoleWriteLine(1);

	DWORD FilesFound = 0;
	vector<SearchResult*>* SearchResults = NULL;

	BOOL SearchSucceeded = SearchContentMetaFiles(FilesFound, SearchResults);

	if (!SearchSucceeded) {
		ConsoleWriteLine(L"     Search failed!");
		ConsoleWriteLine(2);
		ConsoleWriteLine(L"     B: Exit to dashboard");
		ConsoleWriteLine(3);

		while(1) {

			switch(AwaitButtonPress()) {

			case XINPUT_GAMEPAD_B:
				goto Exit;

			default:
				continue;

			}

		}

	}

	if (FilesFound == 0) {

		ConsoleWriteLine(L"     Search completed. No content packages found");
		ConsoleWriteLine(2);
		ConsoleWriteLine(L"     B: Exit to dashboard");
		ConsoleWriteLine(3);

		while(1) {

			switch(AwaitButtonPress()) {

			case XINPUT_GAMEPAD_B:
				goto Exit;

			default:
				continue;

			}

		}

	}

	DWORD PrintBufferSize = (MAX_PATH * sizeof(WCHAR));
	PWCHAR PrintBuffer = (PWCHAR)malloc(PrintBufferSize);
	ZeroMemory(PrintBuffer, PrintBufferSize);

	swprintf(PrintBuffer, L"     Search completed. Found %d folders containing a total of %d content packages", SearchResults->size(), FilesFound);
	ConsoleWriteLine(PrintBuffer);
	ConsoleWriteLine(2);

	ZeroMemory(PrintBuffer, PrintBufferSize);
	swprintf(PrintBuffer, L"     A: Sign all content packages (%d files)", FilesFound);

	ConsoleWriteLine(PrintBuffer);
	ConsoleWriteLine(L"     Y: Sign invalid content packages only (skip already valid content)");
	ConsoleWriteLine(L"     B: Exit to dashboard");
	ConsoleWriteLine(3);

	BOOL OnlySignIfInvalid = FALSE;

	while(1) {

		switch(AwaitButtonPress()) {

			case XINPUT_GAMEPAD_A:
				goto SignFiles;

			case XINPUT_GAMEPAD_Y:
				OnlySignIfInvalid = TRUE;
				goto SignFiles;

			case XINPUT_GAMEPAD_B:
				FreeSearchResults(SearchResults);
				goto Exit;

			default:
				continue;

		}

	}


SignFiles:

	BOOL Error_FileAccess = FALSE;
	BOOL Error_BadFileSize = FALSE;	
	BOOL Error_BadHeaderSize = FALSE;
	BOOL Error_BadMagic = FALSE;

	BOOL Warn_SkippedBecauseValid = FALSE;
	BOOL Warn_IsSimpleContent = FALSE;

	DWORD NumFilesSigned = 0;
	DWORD NumFilesFailed = 0;
	DWORD NumFilesSkipped = 0;

	vector<PWCHAR>* WarningSimpleContentFiles = new vector<PWCHAR>;

	ConsoleWriteLine(L"     Signing content packages...");
	ConsoleWriteLine(2);

	for(DWORD SearchResultIndex = 0; SearchResultIndex < SearchResults->size(); SearchResultIndex++) {

		SearchResult* CurrentSearchResult = SearchResults->at(SearchResultIndex);
		if (CurrentSearchResult->FileCount == 0)
			continue;

		SetAlternateTitleId(CurrentSearchResult->TitleId);

		CHAR AlternateTitleDriveLetter = { 0 };
		XMountAlternateTitle(TitleDataRegion, CurrentSearchResult->TitleId, &AlternateTitleDriveLetter);

		for(DWORD FileIndex = 0; FileIndex < CurrentSearchResult->Files->size(); FileIndex++) {

			PCHAR CurrentFile = CurrentSearchResult->Files->at(FileIndex);

			if (!SignContentMeta(CurrentFile, CurrentSearchResult->TitleId, OnlySignIfInvalid,
				Error_FileAccess, Error_BadFileSize, Error_BadHeaderSize, Error_BadMagic, Warn_SkippedBecauseValid, Warn_IsSimpleContent))
			{
				if (Error_FileAccess || Error_BadFileSize || Error_BadHeaderSize || Error_BadMagic)
					NumFilesFailed++;

				if (Warn_SkippedBecauseValid)
					NumFilesSkipped++;

			}
			else {

				NumFilesSigned++;

				if (Warn_IsSimpleContent && (strlen(CurrentFile) > 0)) {

					PWCHAR File = (PWCHAR)malloc(PrintBufferSize);
					ZeroMemory(File, PrintBufferSize);
					DWORD FileLen = 0;					

					PCHAR FileNameStrippedRoot = strchr(CurrentFile, '\\');
					StringToUnicode((FileNameStrippedRoot + 1), File, FileLen);

					ZeroMemory(PrintBuffer, PrintBufferSize);
					swprintf(PrintBuffer, L"     - E:\\TDATA\\%.08X\\%s", CurrentSearchResult->TitleId, File);

					ZeroMemory(File, PrintBufferSize);
					wcscpy(File, PrintBuffer);

					WarningSimpleContentFiles->push_back(File);
				}

			}

		}

		XUnmountAlternateTitle(AlternateTitleDriveLetter);
	}

	FreeSearchResults(SearchResults);

	if (WarningSimpleContentFiles->size() > 0) {

		ConsoleWriteLine(L"     WARNING: The following content packages are \"simple content\" and may");
		ConsoleWriteLine(L"     continue to display as \"corrupted\" content even after being signed"); //I really need to add line wrapping to the console UI...   
		ConsoleWriteLine(1);

		for(DWORD i = 0; i < WarningSimpleContentFiles->size(); i++) {

			PWCHAR File = WarningSimpleContentFiles->at(i);
			ConsoleWriteLine(File);
			free(File);
		}

		ConsoleWriteLine(2);
	}

	WarningSimpleContentFiles->clear();
	delete WarningSimpleContentFiles;

	ZeroMemory(PrintBuffer, PrintBufferSize);
	swprintf(PrintBuffer, L"     Signing complete. %d files signed, %d failed, %d skipped", NumFilesSigned, NumFilesFailed, NumFilesSkipped);   
	ConsoleWriteLine(PrintBuffer);

	ConsoleWriteLine(2);
	ConsoleWriteLine(L"     B: Exit to dashboard");
	ConsoleWriteLine(3);

	while(1) {

		switch(AwaitButtonPress()) {

			case XINPUT_GAMEPAD_B:
				goto Exit;

			default:
				continue;

		}

	}

Exit:

	if (PrintBuffer)
		free(PrintBuffer);

	if (ConsoleIsInitialized()) {

		ConsoleDestroy();

		while(ConsoleIsInitialized())
			Sleep(20);
	}

	HalReturnToFirmware(HalQuickRebootRoutine);
	Sleep(INFINITE);

}

BOOL SearchContentMetaFiles(DWORD &TotalFiles, vector<SearchResult*>* &Results) {

	/*
	1. Mount E:\ partition
	2. Search E:\TDATA\ for valid TitleId folders and build list of TitleId folders
	3. Loop all TitleId's, overwrite AlternateTitleId in XBE header memory with current TitleId from loop, and mount AlternateTitle drive W:\
	4. Search W:\ for $C and $U folders
	5. If W:\$C exists, search W:\$C for valid OfferId folders and build list of OfferId folders
	6. Grab ContentMeta.xbx from all OfferId folders in W:\$C if exists
	7. Grab ContentMeta.xbx from W:\$U if exists
	*/

	if (!MountPartition(PartitionPath, PartitionLetter)) {
		if (ConsoleIsInitialized())
			ConsoleWriteLine(L"     ERROR: Failed to mount E:\\ partition!");

		return FALSE;
	}

	WIN32_FIND_DATA SearchContext;
	HANDLE SearchHandle;

	PCHAR SearchCriteria = (PCHAR)malloc(MAX_PATH);
	ZeroMemory(SearchCriteria, MAX_PATH);

	sprintf(SearchCriteria, "%c:\\TDATA\\*", PartitionLetter);

	if ((SearchHandle = FindFirstFile(SearchCriteria, &SearchContext)) == INVALID_HANDLE_VALUE) {
		if (ConsoleIsInitialized())
			ConsoleWriteLine(L"     ERROR: Failed to access E:\\ or E:\\TDATA is empty");

		free(SearchCriteria);
		return FALSE;
	}

	BYTE* TitleIds = (BYTE*)malloc((2048 * 4)); // there's ~1000 games for the Xbox (+ breathing room + homebrew titles)
	ZeroMemory(TitleIds, (2048 * 4));
	DWORD CurrentTitleId = 0;
	DWORD TitleIdCount = 0;

	if ((strlen(SearchContext.cFileName) == 8) && 
		ValidHexString(SearchContext.cFileName, 8) && 
		((SearchContext.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)) 
	{

		CopyHexToBytes(SearchContext.cFileName, 8, (BYTE*)&CurrentTitleId, TRUE); // convert hex to uint32, reverse to little endian

		if (CurrentTitleId != 0 && CurrentTitleId != 0xFFFFFFFF) {
			*(DWORD*)(TitleIds + (TitleIdCount * 4)) = CurrentTitleId; //store title id
			TitleIdCount++;
		}
	}

	while(FindNextFile(SearchHandle, &SearchContext)) {

		if ((strlen(SearchContext.cFileName) == 8) && 
			ValidHexString(SearchContext.cFileName, 8) &&
			((SearchContext.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
		{
			CopyHexToBytes(SearchContext.cFileName, 8, (BYTE*)&CurrentTitleId, TRUE); // convert hex to uint32, reverse to little endian

			if (CurrentTitleId != 0 && CurrentTitleId != 0xFFFFFFFF) {
				*(DWORD*)(TitleIds + (TitleIdCount * 4)) = CurrentTitleId; //store title id
				TitleIdCount++;
			}
		}
	}

	FindClose(SearchHandle);

	UnmountPartition(PartitionLetter); // if the partition mounted successfully then there _should_ be no reason to check for successful unmount

	if (TitleIdCount == 0) {

		free(SearchCriteria);
		free(TitleIds);

		if (ConsoleIsInitialized())
			ConsoleWriteLine(L"      ERROR: No valid content folders found");

		return FALSE;
	}

	//for some unicode formatting
	PWCHAR PrintBuffer = (PWCHAR)malloc((125 * sizeof(WCHAR)));

	CHAR AlternateTitleDriveLetter = { 0 };

	Results = new vector<SearchResult*>;

	for(DWORD i = 0; i < TitleIdCount; i++) {

		if (GetAlternateTitleId() != *(DWORD*)(TitleIds + (i * 4))) {

			if (AlternateTitleDriveLetter != '\0') {
				XUnmountAlternateTitle(AlternateTitleDriveLetter);
				AlternateTitleDriveLetter = '\0';
			}

			if (*(DWORD*)(TitleIds + (i * 4)) == 0)
				continue;

			SetAlternateTitleId(*(DWORD*)(TitleIds + (i * 4))); //write title id into XBE header to trick XAPI into giving access when asking to mount
			XMountAlternateTitle(TitleDataRegion, *(DWORD*)(TitleIds + (i * 4)), &AlternateTitleDriveLetter); //mount alternate title id, pass T:\\ to force mounting to W:\\ drive
		}

		BOOL ContentFolderExists = FALSE;
		BOOL UpdateFolderExists = FALSE;

		ZeroMemory(SearchCriteria, MAX_PATH);
		sprintf(SearchCriteria, "%c:\\$c", AlternateTitleDriveLetter);

		if ((SearchHandle = FindFirstFile(SearchCriteria, &SearchContext)) != INVALID_HANDLE_VALUE) {

			ContentFolderExists = TRUE;
			FindClose(SearchHandle);
		}

		ZeroMemory(SearchCriteria, MAX_PATH);
		sprintf(SearchCriteria, "%c:\\$u", AlternateTitleDriveLetter);

		if ((SearchHandle = FindFirstFile(SearchCriteria, &SearchContext)) != INVALID_HANDLE_VALUE) {

			UpdateFolderExists = TRUE;
			FindClose(SearchHandle);
		}

		if (!ContentFolderExists && !UpdateFolderExists)
			continue;


		//begin file search

		SearchResult* Result = new SearchResult;
		Result->TitleId = *(DWORD*)(TitleIds + (i * 4));
		Result->FileCount = 0;
		Result->Files = new vector<PCHAR>;

		if (ContentFolderExists) {

			ZeroMemory(SearchCriteria, MAX_PATH);
			sprintf(SearchCriteria, "%c:\\$c\\ContentMeta.xbx", AlternateTitleDriveLetter);

			//check if a ContentMeta file exists in the root of $C (not usually)
			if ((SearchHandle = FindFirstFile(SearchCriteria, &SearchContext)) != INVALID_HANDLE_VALUE) {

				PCHAR File = (PCHAR)malloc(MAX_PATH);
				ZeroMemory(File, MAX_PATH);

				sprintf(File, "%c:\\$c\\ContentMeta.xbx", AlternateTitleDriveLetter);

				Result->Files->push_back(File);
				Result->FileCount++;

				FindClose(SearchHandle);
			}

			//check for offerid folders

			PCHAR OfferIds = (PCHAR)malloc(16 * 100); //allocate for 100 offerid folders (overkill)
			DWORD OfferIdsCount = 0;

			ZeroMemory(SearchCriteria, MAX_PATH);
			sprintf(SearchCriteria, "%c:\\$c\\*", AlternateTitleDriveLetter);

			if ((SearchHandle = FindFirstFile(SearchCriteria, &SearchContext)) != INVALID_HANDLE_VALUE) {

				if ((strlen(SearchContext.cFileName) == 16) && 
					ValidHexString(SearchContext.cFileName, 16) &&
					((SearchContext.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)) 
				{

					sprintf((OfferIds + OfferIdsCount), SearchContext.cFileName);
					OfferIdsCount++;			
				}

				while(FindNextFile(SearchHandle, &SearchContext)) {

					if ((strlen(SearchContext.cFileName) == 16) && 
						ValidHexString(SearchContext.cFileName, 16) &&
						((SearchContext.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)) 

					{
						sprintf((OfferIds + (OfferIdsCount * 16)), SearchContext.cFileName);
						OfferIdsCount++;
					}

				}

				FindClose(SearchHandle);

			}

			//search for ContentMeta.xbx files within offerid folders

			if (OfferIdsCount > 0) {

				for(DWORD i = 0; i < OfferIdsCount; i++) {

					ZeroMemory(SearchCriteria, MAX_PATH);
					sprintf(SearchCriteria, "%c:\\$c\\%.16s\\ContentMeta.xbx", AlternateTitleDriveLetter, (OfferIds + (i * 16)));

					if ((SearchHandle = FindFirstFile(SearchCriteria, &SearchContext)) != INVALID_HANDLE_VALUE) {

						PCHAR File = (PCHAR)malloc(MAX_PATH);
						ZeroMemory(File, MAX_PATH);

						sprintf(File, "%c:\\$c\\%.16s\\ContentMeta.xbx", AlternateTitleDriveLetter, (OfferIds + (i * 16)));

						Result->Files->push_back(File);
						Result->FileCount++;

						FindClose(SearchHandle);

					}

				}

			}

			free(OfferIds);

		}

		if (UpdateFolderExists) {

			ZeroMemory(SearchCriteria, MAX_PATH);
			sprintf(SearchCriteria, "%c:\\$u\\ContentMeta.xbx", AlternateTitleDriveLetter);

			if ((SearchHandle = FindFirstFile(SearchCriteria, &SearchContext)) != INVALID_HANDLE_VALUE) {

				PCHAR File = (PCHAR)malloc(MAX_PATH);
				ZeroMemory(File, MAX_PATH);

				sprintf(File, "%c:\\$u\\ContentMeta.xbx", AlternateTitleDriveLetter);

				Result->Files->push_back(File);
				Result->FileCount++;

				FindClose(SearchHandle);
			}

		}

		Results->push_back(Result);
		TotalFiles += Result->FileCount;
	}

	if (AlternateTitleDriveLetter != '\0')
		XUnmountAlternateTitle(AlternateTitleDriveLetter);

	free(SearchCriteria);
	free(TitleIds);
	free(PrintBuffer);

	return TRUE;
}

VOID FreeSearchResults(vector<SearchResult*>* &Results) {

	if (!Results)
		return;

	for(DWORD result_index = 0; result_index < Results->size(); result_index++) {

		SearchResult* result = Results->at(result_index);

		if (result) {
			if (result->Files) {

				//free file path strings
				for(DWORD file_index = 0; file_index < result->Files->size(); file_index++)
					if (result->Files->at(file_index))
						free(result->Files->at(file_index));

				//free file path strings list
				result->Files->clear();
				delete result->Files;
			}

			//free search result entry
			delete result;
		}		

	}

	//free search result entries list
	Results->clear();
	delete Results;
}

BOOL SignContentMeta(PCHAR File, DWORD TitleId, BOOL OnlySignIfInvalid, 
					 BOOL &FailedToAccessFile, BOOL &BadFileSize, BOOL &BadHeaderSize, 
					 BOOL &WasInvalidMagic, BOOL &WasSkippedBecauseValid, BOOL &WasSimpleContentMeta) 
{

	FailedToAccessFile = FALSE;
	BadFileSize = FALSE;
	WasInvalidMagic = FALSE;
	WasSkippedBecauseValid = FALSE;
	WasSimpleContentMeta = FALSE;

	if (!File)
		return FALSE;

	const DWORD MinimumHeaderSize = 0x6C;
	BOOL UseTitleIdFromMetadata = (TitleId == 0 || TitleId == 0xFFFFFFFF);

	HANDLE FileHandle = NULL;
	if ((FileHandle = CreateFile(File, GENERIC_ALL, GENERIC_ALL, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE) {

		FailedToAccessFile = TRUE;
		return FALSE;
	}

	DWORD FileSize = GetFileSize(FileHandle, NULL);
	if (FileSize < MinimumHeaderSize) {

		BadFileSize = TRUE;
		CloseHandle(FileHandle);
		return FALSE;
	}

	PBYTE FileBuffer = (PBYTE)malloc(FileSize);

	DWORD BytesRead = 0;
	if (!ReadFile(FileHandle, FileBuffer, FileSize, &BytesRead, NULL) || BytesRead != FileSize) {

		FailedToAccessFile = TRUE;
		CloseHandle(FileHandle);
		free(FileBuffer);
		return FALSE;
	}

	if (*(DWORD*)(FileBuffer + 0x14) != XCSFMagic) {

		WasInvalidMagic = TRUE;
		CloseHandle(FileHandle);
		free(FileBuffer);
		return FALSE;
	}

	DWORD HeaderSize = *(DWORD*)(FileBuffer + 0x18);
	if (HeaderSize < MinimumHeaderSize) {

		BadHeaderSize = TRUE;
		CloseHandle(FileHandle);
		free(FileBuffer);
		return FALSE;
	}

	DWORD InternalTitleId = 0;
	if (UseTitleIdFromMetadata)
		InternalTitleId = *(DWORD*)(FileBuffer + 0x24);

	PBYTE Signature = (PBYTE)malloc(SHA_DIGEST_SIZE);
	CalculateContentHeaderSignature((UseTitleIdFromMetadata ? InternalTitleId : TitleId), (FileBuffer + 0x14), (HeaderSize - 0x14), Signature);

	BOOL SignaturesMatch = (memcmp(FileBuffer, Signature, SHA_DIGEST_SIZE) == 0);

	if (OnlySignIfInvalid && SignaturesMatch) {

		WasSkippedBecauseValid = TRUE;
		CloseHandle(FileHandle);
		free(FileBuffer);
		free(Signature);
		return FALSE;
	}

	SetFilePointer(FileHandle, 0, NULL, FILE_BEGIN);

	DWORD BytesWritten = 0;
	if (!WriteFile(FileHandle, Signature, SHA_DIGEST_SIZE, &BytesWritten, NULL) || BytesWritten != SHA_DIGEST_SIZE) {

		FailedToAccessFile = TRUE;
		CloseHandle(FileHandle);
		free(FileBuffer);
		free(Signature);
		return FALSE;
	}

	FlushFileBuffers(FileHandle);
	CloseHandle(FileHandle);

	free(FileBuffer);
	free(Signature);

	if ((strstr(File, "$c") != NULL) || (strstr(File, "$C") != NULL)) {

		if (HeaderSize == MinimumHeaderSize)
			WasSimpleContentMeta = TRUE;
	}

	return TRUE;
}

BOOL CalculateContentHeaderSignature(DWORD TitleId, PBYTE buffer, DWORD length, PBYTE result) {

	if (buffer && length > 0 && result) {

		PBYTE ContentKey = (PBYTE)malloc(SHA_DIGEST_SIZE);

		//calc content key (HMAC SHA1 hash of TitleId bytes using XboxHDKey as the key)
		XcHMAC(&XboxHDKey[0], 0x10, (PBYTE)&TitleId, 4, NULL, NULL, ContentKey);

		//calc signature (HMAC SHA1 hash of supplied data using "content key" as the key)
		XcHMAC(ContentKey, SHA_DIGEST_SIZE, buffer, length, NULL, NULL, result);

		free(ContentKey);

		return TRUE;
	}

	return FALSE;
}

DWORD GetAlternateTitleId() {

	//NOTE: only using the first alternate title id, it's possible to use up to 16 total

	//NOTE(feudalnate): might be a good idea to cache the alternate title id's address instead of fetching every time

	DWORD CertificateAddress = *(DWORD*)(XBEBaseAddress + 0x118); //get certificate address from XBE header
	DWORD AlternateTitleIdsAddress = (CertificateAddress + 0x5C); //alternate titleids are located 0x5C from beginning of certificate (in all cert. revisions)

	return *(DWORD*)AlternateTitleIdsAddress;
}

VOID SetAlternateTitleId(DWORD AlternateTitleId) {

	DWORD CertificateAddress = *(DWORD*)(XBEBaseAddress + 0x118);
	DWORD AlternateTitleIdsAddress = (CertificateAddress + 0x5C);

	*(DWORD*)AlternateTitleIdsAddress = AlternateTitleId;
}

BOOL CheckXBEHeader() {

	if (MemoryPageWritable((LPVOID)XBEBaseAddress))
		if (*(DWORD*)XBEBaseAddress == XBEMagic)
			return TRUE;

	return FALSE;
}

BOOL MemoryPageWritable(LPVOID Address) {

	/*
	WinNT memory page protection flags (one or more will _always_ be set)

	PAGE_NOACCESS			0x01
	PAGE_READONLY			0x02
	PAGE_READWRITE			0x04
	PAGE_WRITECOPY			0x08
	PAGE_EXECUTE			0x10
	PAGE_EXECUTE_READ		0x20
	PAGE_EXECUTE_READWRITE	0x40
	PAGE_EXECUTE_WRITECOPY	0x80
	PAGE_GUARD				0x100
	PAGE_NOCACHE			0x200
	PAGE_WRITECOMBINE		0x400
	*/

	DWORD Flags = XQueryMemoryProtect(Address);

	if ( (Flags == 0) || ((Flags & PAGE_GUARD) == PAGE_GUARD) || ((Flags & PAGE_NOACCESS) == PAGE_NOACCESS) )
		return FALSE;

	if ( ((Flags & PAGE_READWRITE) == PAGE_READWRITE) || ((Flags & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE) )
		return TRUE;

	return FALSE;
}

BOOL MountPartition(PCHAR PartitionPath, CHAR PartitionLetter) {

	if (PartitionPath && (PartitionLetter >= 'A' || PartitionLetter <= 'Z' || PartitionLetter >= 'a' || PartitionLetter <= 'z')) {

		PCHAR DevicePath = (PCHAR)malloc(MAX_PATH);
		PCHAR MountPath = (PCHAR)malloc(MAX_PATH);

		ZeroMemory(DevicePath, MAX_PATH);
		ZeroMemory(MountPath, MAX_PATH);

		sprintf(DevicePath, "%s", PartitionPath);
		sprintf(MountPath, "\\??\\%c:", PartitionLetter);

		ANSI_STRING DeviceName;
		DeviceName.Buffer = DevicePath;
		DeviceName.Length = strlen(DevicePath);
		DeviceName.MaximumLength = MAX_PATH;

		ANSI_STRING SymbolicLinkName;
		SymbolicLinkName.Buffer = MountPath;
		SymbolicLinkName.Length = strlen(MountPath);
		SymbolicLinkName.MaximumLength = MAX_PATH;

		LONG Result = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName); //returns NTSTATUS

		free(DevicePath);
		free(MountPath);

		return (Result >= 0); //NT_SUCCESS
	}

	return FALSE;
}

BOOL UnmountPartition(CHAR PartitionLetter) {

	if (PartitionLetter >= 'A' || PartitionLetter <= 'Z' || PartitionLetter >= 'a' || PartitionLetter <= 'z') {

		PCHAR MountPath = (PCHAR)malloc(MAX_PATH);
		ZeroMemory(MountPath, MAX_PATH);

		sprintf(MountPath, "\\??\\%c:", PartitionLetter);

		ANSI_STRING SymbolicLinkName;
		SymbolicLinkName.Buffer = MountPath;
		SymbolicLinkName.Length = strlen(MountPath);
		SymbolicLinkName.MaximumLength = MAX_PATH;

		LONG Result = IoDeleteSymbolicLink(&SymbolicLinkName);

		free(MountPath);

		return (Result >= 0);
	}

	return FALSE;
}

DWORD AwaitButtonPress() {

	DWORD Result = UINT_MAX;

	CONST DWORD ControllerCount = 4;
	HANDLE Controller[ControllerCount] = { 0 };

	DWORD LastStatePacketNumber[ControllerCount];
	XINPUT_STATE CurrentState[ControllerCount];

	for(DWORD i = 0; i < ControllerCount; i++) {
		ZeroMemory(&CurrentState[i], sizeof(XINPUT_STATE));
		LastStatePacketNumber[i] = 0;
	}

	DWORD Insertions = 0;
	DWORD Removals = 0;

	for(DWORD i = 0; i < ControllerCount; i++)
		Controller[i] = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);

	while(true) {

		//check if controllers were unplugged/plugged in
		if (XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &Insertions, &Removals)) {

			//wait for enumeration to complete
			while(XGetDeviceEnumerationStatus() != XDEVICE_ENUMERATION_IDLE)
				Sleep(15);

			//close current device handles
			for(DWORD i = 0; i < ControllerCount; i++)
				if (Controller[i]) XInputClose(Controller[i]);

			//open new device handles
			for(DWORD i = 0; i < ControllerCount; i++)
				Controller[i] = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);

		}

		//poll all controllers
		for (DWORD i = 0; i < ControllerCount; i++) {

			//get current input state
			if (Controller[i] && XInputGetState(Controller[i], &CurrentState[i]) == ERROR_SUCCESS) {

				//if packet id changed then process input
				if (CurrentState[i].dwPacketNumber != LastStatePacketNumber[i]) {

					LastStatePacketNumber[i] = CurrentState[i].dwPacketNumber;

					//checking for button presses only

					//START
					if ((CurrentState[i].Gamepad.wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START) {
						Result = XINPUT_GAMEPAD_START;
						break;
					}

					//BACK
					if ((CurrentState[i].Gamepad.wButtons & XINPUT_GAMEPAD_BACK) == XINPUT_GAMEPAD_BACK) {
						Result = XINPUT_GAMEPAD_BACK;
						break;
					}

					//LEFT STICK (CLICK)
					if ((CurrentState[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) == XINPUT_GAMEPAD_LEFT_THUMB) {
						Result = XINPUT_GAMEPAD_LEFT_THUMB;
						break;
					}


					//RIGHT STICK (CLICK)
					if ((CurrentState[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) == XINPUT_GAMEPAD_RIGHT_THUMB) {
						Result = XINPUT_GAMEPAD_RIGHT_THUMB;
						break;
					}

					//A BUTTON
					if (CurrentState[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_A] > XINPUT_GAMEPAD_MAX_CROSSTALK) {
						Result = XINPUT_GAMEPAD_A;
						break;
					}

					//B BUTTON
					if (CurrentState[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_B] > XINPUT_GAMEPAD_MAX_CROSSTALK) {
						Result = XINPUT_GAMEPAD_B;
						break;
					}

					//X BUTTON
					if (CurrentState[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_X] > XINPUT_GAMEPAD_MAX_CROSSTALK) {
						Result = XINPUT_GAMEPAD_X;
						break;
					}

					//Y BUTTON
					if (CurrentState[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_Y] > XINPUT_GAMEPAD_MAX_CROSSTALK) {
						Result = XINPUT_GAMEPAD_Y;
						break;
					}

					//BLACK BUTTON
					if (CurrentState[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_BLACK] > XINPUT_GAMEPAD_MAX_CROSSTALK) {
						Result = XINPUT_GAMEPAD_BLACK;
						break;
					}

					//WHITE BUTTON
					if (CurrentState[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_WHITE] > XINPUT_GAMEPAD_MAX_CROSSTALK) {
						Result = XINPUT_GAMEPAD_WHITE;
						break;
					}

				}

			}

		}

		if (Result != UINT_MAX)
			break;

		Sleep(100); //prevent spam
	}

	//close handles
	for(int i = 0; i < ControllerCount; i++)
		if (Controller[i]) XInputClose(Controller[i]);

	return Result;
}