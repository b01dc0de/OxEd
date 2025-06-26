#include "OxEd.h"

float OxEd_Platform_win32::fScale = 1.0f;
HWND OxEd_Platform_win32::hWindow = nullptr;
HINSTANCE OxEd_Platform_win32::_hInst = nullptr;
//HINSTANCE OxEd_Platform_win32::_hPrevInst;
//PSTR OxEd_Platform_win32::_CmdLine;
//int OxEd_Platform_win32::_WndShow;

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

void OxEd_Platform_win32::OpenFile(FileContentsT& OutContents)
{
    char* FileNameBuffer = new char[FileContentsT::MaxNameSize]{};

    OPENFILENAMEA DialogState = {};
    DialogState.lStructSize = sizeof(OPENFILENAMEA);
    DialogState.hwndOwner = hWindow;
    DialogState.hInstance = nullptr;
    DialogState.lpstrFilter = nullptr;
    DialogState.lpstrCustomFilter = nullptr;
    DialogState.nFilterIndex = 0;
    DialogState.lpstrFile = FileNameBuffer;
    DialogState.nMaxFile = FileContentsT::MaxNameSize;
    DialogState.lpstrFileTitle = nullptr;
    DialogState.lpstrInitialDir = nullptr;
    DialogState.lpstrTitle = nullptr;
    DialogState.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    DialogState.lpstrDefExt = nullptr;
    DialogState.lCustData = 0;
    DialogState.lpfnHook = nullptr;
    BOOL bResult = GetOpenFileNameA(&DialogState);
    if (bResult)
    {
		// User hit 'OK'
        ReadFileContents(FileNameBuffer, OutContents);
		OutContents.Name = FileNameBuffer;
    }
    else
    {
		// User canceled dialog, or other error occured
        //DWORD ExError = CommDlgExtendedError();
        //DebugBreak();
    }
}

void OxEd_Platform_win32::Tick()
{
	MSG Msg = {};
	int MsgCount = 0;
    while (PeekMessageA(&Msg, hWindow, 0, 0, PM_REMOVE) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessageA(&Msg);
		MsgCount++;
    }
	UpdateWindow(hWindow);
}

bool OxEd_Platform_win32::Init()
{
#if OXED_CONFIG_DEBUG()
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif // OXED_CONFIG_DEBUG()

	WNDCLASSEXA WndClass = {};
	WndClass.cbSize = sizeof(WNDCLASSEXA);
	WndClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.hInstance = _hInst;
	WndClass.lpszClassName = APPNAME();

	RegisterClassExA(&WndClass);

	RECT WndRect = { 0, 0, (LONG)WinResX, (LONG)WinResY};
	UINT WndStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
	UINT WndExStyle = 0;
	AdjustWindowRectEx(&WndRect, WndStyle, FALSE, WndExStyle);

    hWindow = CreateWindowExA(
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
		_hInst,
		nullptr
	);
	if (hWindow)
	{
        ShowWindow(hWindow, SW_SHOWNORMAL);
        UpdateWindow(hWindow);
	}
	return hWindow != nullptr;
}

bool OxEd_Platform_win32::Term()
{
	return true;
}

void OxEd_Platform_win32::ImGui_Init()
{
    ImGui_ImplWin32_EnableDpiAwareness();
    fScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
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
    OxEd_Platform_win32::_hInst = hInst;
    //OxEd_Platform_win32::_hPrevInst = hPrevInst;
    //OxEd_Platform_win32::_CmdLine = CmdLine;
    //OxEd_Platform_win32::_WndShow = WndShow;

	OxEdMain();

	return 0;
}

