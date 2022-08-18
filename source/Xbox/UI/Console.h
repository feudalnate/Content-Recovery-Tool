#ifndef XBOX_CONSOLE
#define XBOX_CONSOLE

/*
	NOTE(feudalnate):

	Current issues:

	1) No Write() support
	2) Not parsing \n or \r
	3) No option to set safe-area
	4) Acts as an app, no option to set console as a specific region on the screen
	5) No option to pass external font (kind of)
	6) No misc. options to set cursor position, back color, font color, font size, default indent, etc.
	7) *NO LINE WRAPPING!*
	8) No WriteLine with formatted arglist
*/


VOID ConsoleInit(DWORD Width, DWORD Height, WCHAR* TTF_File, DWORD TTF_FontHeight);
VOID ConsoleDestroy();
BOOL ConsoleIsInitialized();
VOID ConsoleWriteLine(DWORD number_of_blank_lines = 1);
VOID ConsoleWriteLine(WCHAR* string);
VOID ConsoleClear();

#endif