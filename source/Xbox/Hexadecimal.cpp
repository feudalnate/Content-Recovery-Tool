#include "Hexadecimal.h"

BOOL ValidHexString(CHAR* string, DWORD length) {

	//this could be done better/faster

	if (!string || length == 0 || (length % 2) != 0)
		return FALSE;

	const CHAR* valid_chars = "0123456789abcdefABCDEF";
	const DWORD valid_chars_len = 22;

	BOOL valid;
	CHAR c, v;
	for(DWORD i = 0; i < length; i++) {

		valid = FALSE;
		c = *(CHAR*)(string + i);

		for(DWORD x = 0; x < valid_chars_len; x++) {

			v = *(CHAR*)(valid_chars + x);

			if (c == v) {
				valid = TRUE;
				break;
			}

		}

		if (!valid)
			return FALSE;

	}

	return TRUE;
}

VOID CopyHexToBytes(CHAR* string, DWORD length, BYTE* buffer, BOOL reverse) {

	if (string && length > 0 && (length % 2) == 0 && buffer) {

		CHAR c;
		BYTE b;

		DWORD buffer_index = 0;

		for(DWORD i = 0; i < length; i += 2) {

			//first nibble
			c = *(CHAR*)(string + i);
			
			if (c >= '0' && c <= '9')
				b = (BYTE)((c - '0') << 4);
			else if (c >= 'A' && c <= 'F')
				b = (BYTE)((c - 'A' + 10) << 4);
			else
				b = (BYTE)((c - 'a' + 10) << 4);

			//second nibble
			c = *(CHAR*)(string + (i + 1));
			
			if (c >= '0' && c <= '9')
				b = (BYTE)(b | (c - '0'));
			else if (c >= 'A' && c <= 'F')
				b = (BYTE)(b | (c - 'A' + 10));
			else
				b = (BYTE)(b | (c - 'a' + 10));

			*(BYTE*)(buffer + buffer_index) = b;
			buffer_index++;
		}

		if (reverse) {

			DWORD x = 0;
			DWORD y = (length / 2);

			while (x < y) {
				b = *(BYTE*)(buffer + x);
				*(BYTE*)(buffer + x) = *(BYTE*)(buffer + (y - 1));
				*(BYTE*)(buffer + (y - 1)) = b;
				x++;
				y--;
			}

		}

	}
}