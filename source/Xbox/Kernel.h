#ifndef KERNEL_IMPORTS
#define KERNEL_IMPORTS

#define SHA_CONTEXT_SIZE 116
#define SHA_DIGEST_SIZE 20

extern "C" 
{
	XBOXAPI UCHAR NTSYSAPI XboxHDKey[0x10];

	typedef struct _STRING 	{
		USHORT Length;
		USHORT MaximumLength;
		PSTR Buffer;
	} STRING, *PSTRING, ANSI_STRING, *PANSI_STRING;

	typedef struct _UNICODE_STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PWSTR Buffer;
	} UNICODE_STRING, *PUNICODE_STRING;

	//unicode buffer size from ansi string size
	XBOXAPI LONG NTAPI RtlMultiByteToUnicodeSize(PULONG BytesInUnicodeString, PCHAR MultiByteString, ULONG BytesInMultiByteString);

	//ansi buffer size from unicode string size
	XBOXAPI LONG NTAPI RtlUnicodeToMultiByteSize(PULONG BytesInMultiByteString, PWSTR UnicodeString, ULONG BytesInUnicodeString);
	
	//convert unicode to ansi
	XBOXAPI LONG NTAPI RtlUnicodeStringToAnsiString(PSTRING DestinationString, PUNICODE_STRING SourceString, BOOLEAN AllocateDestinationString);

	//convert ansi to unicode
	XBOXAPI LONG NTAPI RtlAnsiStringToUnicodeString(PUNICODE_STRING DestinationString, PSTRING SourceString, BOOLEAN AllocateDestinationString);

	//free ansi string struct if allocated by Rtl function
	XBOXAPI VOID NTAPI RtlFreeAnsiString(PANSI_STRING AnsiString);

	//free unicode string struct if allocated by Rtl function
	XBOXAPI VOID NTAPI RtlFreeUnicodeString(PUNICODE_STRING UnicodeString);

	//create new symlink between physical path and virtual drive letter
	XBOXAPI LONG NTAPI IoCreateSymbolicLink(PSTRING SymbolicLinkName, PSTRING DeviceName);

	//remove existing symlink
	XBOXAPI LONG NTAPI IoDeleteSymbolicLink(PSTRING SymbolicLinkName);

	typedef enum _FIRMWARE_REENTRY {

		//halt CPU (don't use this)
		HalHaltRoutine = 0,

		//hard-reset (power cycle)
		HalRebootRoutine = 1,

		//soft-reset (re-init kernel from phase2)
		HalQuickRebootRoutine = 2,

		//debugger soft-reset
		HalKdRebootRoutine = 3,

		//soft-reset into UEM screen (must write error code to SMC first!)
		HalFatalErrorRebootRoutine = 4,

		//here be dragons >:]
		HalMaximumRoutine = 5

	} FIRMWARE_REENTRY;

	XBOXAPI VOID NTAPI HalReturnToFirmware(FIRMWARE_REENTRY Routine); //not for the faint of heart
	
	//sha-1
	XBOXAPI VOID NTAPI XcSHAInit(PBYTE pbSHAContext);
	XBOXAPI VOID NTAPI XcSHAUpdate(PBYTE pbSHAContext, PBYTE pbInput, DWORD dwInputLength);
	XBOXAPI VOID NTAPI XcSHAFinal(PBYTE pbSHAContext, PBYTE pbDigest);

    //hmac-sha1 (input2/inputlen2 can be null)
	XBOXAPI VOID NTAPI XcHMAC(PBYTE pbKey, DWORD dwKeyLength, PBYTE pbInput, DWORD dwInputLength, PBYTE pbInput2, DWORD dwInputLength2, PBYTE pbDigest);

}

#endif