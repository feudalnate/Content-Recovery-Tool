#include "Unicode.h"

BOOL UnicodeToString(WCHAR* unicode_string, PCHAR &string, DWORD &string_length) {
	
	if (!unicode_string || !string)
		return FALSE;

	USHORT unicode_length_bytes = (wcslen(unicode_string) * sizeof(WCHAR));

	UNICODE_STRING input;
	input.Buffer = unicode_string;
	input.Length = unicode_length_bytes;
	input.MaximumLength = unicode_length_bytes;

	ANSI_STRING output = { 0 };

	if (RtlUnicodeStringToAnsiString(&output, &input, TRUE) >= 0) {

		string_length = strlen(output.Buffer);
		memcpy(string, output.Buffer, string_length);

		RtlFreeAnsiString(&output);

		return TRUE;
	}

	return FALSE;
}

BOOL StringToUnicode(PCHAR string, PWCHAR &unicode_string, DWORD &unicode_string_length) {

	if (!string || !unicode_string)
		return FALSE;

	USHORT string_length_bytes = strlen(string);

	ANSI_STRING input;
	input.Buffer = string;
	input.Length = string_length_bytes;
	input.MaximumLength = string_length_bytes;

	UNICODE_STRING output = { 0 };

	if (RtlAnsiStringToUnicodeString(&output, &input, TRUE) >= 0) {

		unicode_string_length = wcslen(output.Buffer);
		memcpy(unicode_string, output.Buffer, (unicode_string_length * sizeof(WCHAR)));

		RtlFreeUnicodeString(&output);

		return TRUE;
	}

    return FALSE;
}