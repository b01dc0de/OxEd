#include "OxEd.h"
#include "Graphics_DX11.h"

#if UNICODE
#define APPNAME() (L"OxEd")
#else
#define APPNAME() ("OxEd")
#endif

// Globals
bool bRunning = false;
HWND hWindow = nullptr;
UINT WinResX = 1600U;
UINT WinResY = 900U;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND InitWindow(HINSTANCE hInstance, int Width, int Height)
{
	WNDCLASSEX WndClass = {};
	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.hInstance = hInstance;
	WndClass.lpszClassName = APPNAME();

	RegisterClassEx(&WndClass);

	RECT WndRect = { 0, 0, (LONG)Width, (LONG)Height};
	UINT WndStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
	UINT WndExStyle = WS_EX_OVERLAPPEDWINDOW;
	AdjustWindowRectEx(&WndRect, WndStyle, FALSE, WndExStyle);

	HWND NewWindow = CreateWindowEx(
		WndExStyle,
		APPNAME(),
		APPNAME(),
		WndStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WndRect.right - WndRect.left,
		WndRect.bottom - WndRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	return NewWindow;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch (uMsg)
	{
		case WM_KEYUP:
		{
			if (VK_ESCAPE == wParam)
			{
				bRunning = false;
			}
		} break;
		case WM_CLOSE:
		{
			bRunning = false;
		} break;
		default:
		{
			Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		} break;
	}
	
	return Result;
}

int WindowMsgLoop(HWND InWindow)
{
	MSG Msg = {};
	int MsgCount = 0;
    while (PeekMessage(&Msg, InWindow, 0, 0, PM_REMOVE) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
		MsgCount++;
    }
	return MsgCount;
}

int WINAPI WinMain_EmptyWindow(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	(void)hPrevInst;
	(void)CmdLine;
	if (HWND hWnd = InitWindow(hInst, WinResX, WinResY))
	{
		hWindow = hWnd;

		ShowWindow(hWindow, WndShow);

		bRunning = true;
		while (bRunning)
		{
			WindowMsgLoop(hWindow);
		}
	}
	return 0;
}
int WINAPI WinMain_DX11_Demo(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	(void)hPrevInst;
	(void)CmdLine;
	if (HWND hWnd = InitWindow(hInst, WinResX, WinResY))
	{
		hWindow = hWnd;

		HRESULT Result = InitGraphics();
		if (Result != S_OK)
		{
			DebugBreak();
		}

		ShowWindow(hWindow, WndShow);

		bRunning = true;
		while (bRunning)
		{
			WindowMsgLoop(hWindow);
			UpdateWindow(hWindow);
			Draw();
		}
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	int Result = 0;

	static const int Project_Win32_EmptyWindow= 0;
	static const int Project_Win32_DX11_Demo = 1;

	static int BuildProject = Project_Win32_DX11_Demo;
	switch (BuildProject)
	{
		case Project_Win32_EmptyWindow:
		{
			Result = WinMain_EmptyWindow(hInst, hPrevInst, CmdLine, WndShow);
		} break;
		case Project_Win32_DX11_Demo:
		{
			Result = WinMain_DX11_Demo(hInst, hPrevInst, CmdLine, WndShow);
		} break;
		default:
		{
			Result = 1;
			DebugBreak();
		} break;
	}

	return Result;
}

