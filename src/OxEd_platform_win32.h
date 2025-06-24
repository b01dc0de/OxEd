#ifndef OXED_WIN32_H
#define OXED_WIN32_H

// Win32 headers:
//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#if _DEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif // _DEBUG

// Dear ImGui:
#include "imgui/backends/imgui_impl_win32.h"

struct OxEd_Platform_win32
{
    static const char* Name() { return "PlatformWin32"; }
    static void OpenFile(FileContentsT& OutContents);
    static void Tick();
    static bool Init();
    static bool Term();
    static void ImGui_Init();
    static void ImGui_Term();
    static void ImGui_NewFrame();

    static HWND hWindow;
    static HINSTANCE _hInst;
    static HINSTANCE _hPrevInst;
    static PSTR _CmdLine;
    static int _WndShow;
};

using OxEd_PlatformT = OxEd_Platform_win32;

#define APPNAME() ("OxEd")
#define ASSERT(Exp) if (!(Exp)) { DebugBreak(); }

#endif // OXED_WIN32_H

