#include <xtl.h>
#define XFONT_TRUETYPE //CDX font import support
#include <xfont.h>

#include "Console.h"
#include "ConsoleFont.h"

//private locals
LPDIRECT3D8 DeviceEnumerator;
LPDIRECT3DDEVICE8 GraphicsDevice;
D3DPRESENT_PARAMETERS Parameters;
LPDIRECT3DSURFACE8 BackBuffer;

D3DCOLOR BACKGROUND_COLOR = D3DCOLOR_XRGB(0, 0, 0);
D3DCOLOR FONT_COLOR = D3DCOLOR_XRGB(255, 255, 255);

XFONT* Font;
DWORD FontHeight;
DWORD FontWidth;

BOOL ConsoleInitialized = FALSE;

DWORD ConsoleWidth;
DWORD ConsoleHeight;

DWORD ConsoleMaxRows;
DWORD ConsoleMaxColumns;

DWORD ConsoleCurrentRow = 0;
DWORD ConsoleCurrentColumn = 0;

WCHAR** ConsoleCharacterMatrix;

//private defines
VOID RenderFrame();
VOID ShiftRowsUp();
VOID ClearScreen(D3DCOLOR Color, BOOL PresentNewScene);

VOID ConsoleInit(DWORD Width, DWORD Height, WCHAR* TTF_File, DWORD TTF_FontHeight) {

	if (ConsoleInitialized || Width == 0 || Height == 0)
		return;

	ConsoleWidth = Width;
	ConsoleHeight = Height;

	//set DirectX params
	ZeroMemory(&Parameters, sizeof(Parameters));
	Parameters.BackBufferWidth = ConsoleWidth;
	Parameters.BackBufferHeight = ConsoleHeight;
	Parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
	Parameters.BackBufferCount = 1;
	Parameters.EnableAutoDepthStencil = TRUE;
	Parameters.AutoDepthStencilFormat = D3DFMT_D24S8;
	Parameters.SwapEffect = D3DSWAPEFFECT_COPY;
	Parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
	//Parameters.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR; //makes font look like garbage

	//grab enumerator, grab device context, disable fancy lighting
	DeviceEnumerator = Direct3DCreate8(D3D_SDK_VERSION);
	DeviceEnumerator->CreateDevice(NULL, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Parameters, &GraphicsDevice);
	GraphicsDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	//grab frame buffer
	GraphicsDevice->GetBackBuffer(NULL, NULL, &BackBuffer);

	//open font, set color

	if (TTF_File && TTF_FontHeight > 0) {

		if (XFONT_OpenTrueTypeFont(TTF_File, 0x4000, &Font) != S_OK)
			goto open_default;

		Font->SetTextHeight(TTF_FontHeight);
	}
	else {

open_default:
		XFONT_OpenBitmapFontFromMemory(&Tahoma12Normal, sizeof(Tahoma12Normal), &Font);
	}

	Font->SetTextColor(FONT_COLOR);

	//do font measurements

	/*
		Usually capital W or the @ symbol will be the widest characters in a font
		Measure both and use which ever is widest in pixels as the font width
	*/
	unsigned int A;
	unsigned int B;

	Font->GetTextExtent(L"W", -1, &A);
	Font->GetTextExtent(L"@", -1, &B);

	FontWidth = (DWORD)(A >= B ? A : B);

	Font->GetFontMetrics(&A, &B); //get height + any underhang from characters like lower case g or q
	FontHeight = (DWORD)(A + B); //add height + underhang to prevent any text smushing

	ConsoleMaxColumns = (ConsoleWidth / FontWidth) * sizeof(WCHAR); // !!!BUG!!! the * sizeof(WCHAR) is a hack, find why column width isnt calculating properly (repro: remove hack & print long string)
	ConsoleMaxRows = (ConsoleHeight / FontHeight);

	ConsoleCharacterMatrix = new WCHAR*[ConsoleMaxRows];
	for(DWORD i = 0; i < ConsoleMaxRows; i++) {
		ConsoleCharacterMatrix[i] = new WCHAR[ConsoleMaxColumns + 1]; //+ 1 for null term
		ZeroMemory(ConsoleCharacterMatrix[i], ((ConsoleMaxColumns * sizeof(WCHAR)) + sizeof(WCHAR)));
	}

	//clear screen to black
	ClearScreen(BACKGROUND_COLOR, TRUE);

	ConsoleInitialized = TRUE;
}

VOID ConsoleDestroy() {
	
	if (!ConsoleInitialized)
		return;

	if (ConsoleCharacterMatrix) {

		for(DWORD i = 0; i < ConsoleMaxRows; i++)
			if (ConsoleCharacterMatrix[i])
				delete[] ConsoleCharacterMatrix[i];

		delete[] ConsoleCharacterMatrix;
	}

	ClearScreen(BACKGROUND_COLOR, TRUE);

	ConsoleInitialized = FALSE;
}

BOOL ConsoleIsInitialized() {
	return ConsoleInitialized;
}

VOID ConsoleWriteLine(DWORD number_of_blank_lines) {
	
	if (!ConsoleInitialized || number_of_blank_lines <= 0)
		return;

	for(DWORD i = 0; i < number_of_blank_lines; i++) {

		if (ConsoleCurrentRow >= ConsoleMaxRows) {
			ShiftRowsUp();
			ShiftRowsUp();
			ConsoleCurrentRow = (ConsoleMaxRows - 2);
		}

		ConsoleCurrentRow++;

		RenderFrame();
	}
}

VOID ConsoleWriteLine(WCHAR* string) {
	
	if (!ConsoleInitialized || !string)
		return;

	if (ConsoleCurrentRow >= ConsoleMaxRows) {
		ShiftRowsUp();
		ConsoleCurrentRow = (ConsoleMaxRows - 1);
	}

	if (ConsoleCharacterMatrix && ConsoleCharacterMatrix[ConsoleCurrentRow]) {

		DWORD StringLength = wcslen(string);

		if(StringLength > ConsoleMaxColumns)
			wcsncpy(ConsoleCharacterMatrix[ConsoleCurrentRow], string, ConsoleMaxColumns); 
		else
			wcsncpy(ConsoleCharacterMatrix[ConsoleCurrentRow], string, StringLength);
	}
	
	ConsoleCurrentRow++;

	RenderFrame();
}

VOID ConsoleClear() {

	if (!ConsoleInitialized)
		return;

	if (ConsoleCharacterMatrix) {
		for(DWORD i = 0; i < ConsoleMaxRows; i++)
			if (ConsoleCharacterMatrix[i])
				ZeroMemory(ConsoleCharacterMatrix[i], ((ConsoleMaxColumns * sizeof(WCHAR)) + sizeof(WCHAR)));

		ConsoleCurrentRow = 0;
		ConsoleCurrentColumn = 0;
		RenderFrame();
	}

}

VOID RenderFrame() {

	if (!ConsoleInitialized)
		return;

	if (GraphicsDevice && Font && ConsoleCharacterMatrix) {

		GraphicsDevice->BeginScene();
		ClearScreen(BACKGROUND_COLOR, FALSE);

		for(DWORD i = 0; i < ConsoleCurrentRow; i++) //ConsoleMaxRows; i++)
			if (ConsoleCharacterMatrix[i])
				if (*(WCHAR*)ConsoleCharacterMatrix[i] != L'\0')
					Font->TextOut(BackBuffer, ConsoleCharacterMatrix[i], -1, 0, (i * FontHeight));

		GraphicsDevice->EndScene();
		GraphicsDevice->Present(NULL, NULL, NULL, NULL);
	}

}

VOID ShiftRowsUp() {

	if (!ConsoleInitialized)
		return;

	if (ConsoleCharacterMatrix) {
		DWORD DestinationIndex = 0;
		DWORD SourceIndex = 1;
		
		while (SourceIndex < ConsoleMaxRows) {

			wcscpy(ConsoleCharacterMatrix[DestinationIndex], ConsoleCharacterMatrix[SourceIndex]);

			DestinationIndex++;
			SourceIndex++;
		}

		ZeroMemory(ConsoleCharacterMatrix[ConsoleMaxRows - 1], ((ConsoleMaxColumns * sizeof(WCHAR)) + sizeof(WCHAR)));
	}

}

VOID ClearScreen(D3DCOLOR Color, BOOL PresentNewScene) {

	if(ConsoleInitialized && GraphicsDevice) {

		D3DRECT Rect = { 0 };
		Rect.x2 = ConsoleWidth;
		Rect.y2 = ConsoleHeight;

		if (PresentNewScene)
			GraphicsDevice->BeginScene();

		GraphicsDevice->Clear(1, &Rect, D3DCLEAR_TARGET, Color, NULL, NULL);

		if (PresentNewScene) {
			GraphicsDevice->EndScene();
			GraphicsDevice->Present(NULL, NULL, NULL, NULL);
		}
	}
}