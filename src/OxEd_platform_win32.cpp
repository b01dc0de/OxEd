#include "OxEd.h"

HWND hWindow = nullptr;

HINSTANCE _hInst;
HINSTANCE _hPrevInst;
PSTR _CmdLine;
int _WndShow;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND InitWindow(HINSTANCE hInstance, int Width, int Height)
{
#if _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif // _DEBUG

	WNDCLASSEXA WndClass = {};
	WndClass.cbSize = sizeof(WNDCLASSEXA);
	WndClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.hInstance = hInstance;
	WndClass.lpszClassName = APPNAME();

	RegisterClassExA(&WndClass);

	RECT WndRect = { 0, 0, (LONG)Width, (LONG)Height};
	UINT WndStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
	UINT WndExStyle = 0;
	AdjustWindowRectEx(&WndRect, WndStyle, FALSE, WndExStyle);

	HWND NewWindow = CreateWindowExA(
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
	// Dear Imgui:
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) { return true; }

	LRESULT Result = 0;
	switch (uMsg)
	{
		case WM_KEYUP:
		{
			if (VK_ESCAPE == wParam) { bRunning = false; }
		} break;
		case WM_CLOSE:
		{
			bRunning = false;
		} break;
		default:
		{
			Result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
		} break;
	}
	
	return Result;
}

int WindowMsgLoop(HWND InWindow)
{
	MSG Msg = {};
	int MsgCount = 0;
    while (PeekMessageA(&Msg, InWindow, 0, 0, PM_REMOVE) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessageA(&Msg);
		MsgCount++;
    }
	return MsgCount;
}

void OxEd_Tick()
{
}

void OxEd_Platform_win32::Tick()
{
    WindowMsgLoop(hWindow);
    //UpdateWindow(hWindow);
}

bool OxEd_Platform_win32::Init()
{
	HWND hWnd = InitWindow(_hInst, WinResX, WinResY);
	if (hWnd)
	{
        hWindow = hWnd;
        ShowWindow(hWindow, SW_SHOWNORMAL);
	}
	return hWnd != nullptr;
}

bool OxEd_Platform_win32::Term()
{
	return true;
}

void OxEd_Platform_win32::ImGui_Init()
{
    ImGui_ImplWin32_Init(hWindow);
}

void OxEd_Platform_win32::ImGui_Term()
{
    ImGui_ImplWin32_Shutdown();
}

void OxEd_Platform_win32::ImGui_NewFrame()
{
	ImGui_ImplWin32_NewFrame();
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
    _hInst = hInst;
    _hPrevInst = hPrevInst;
    _CmdLine = CmdLine;
    _WndShow = WndShow;
	OxEd_Run();
	return 0;
}



