#ifndef OXED_PLATFORM_H
#define OXED_PLATFORM_H

// Project main:
extern void OxEdMain();

struct OxEd_Platform_dummy
{
    static const char* Name() { return "PlatformDummy"; }
    static float Scale() { return 1.0f; }
    static void Tick() {}
    static bool Init() { return true; }
    static bool Term() { return true; }
    static void ImGui_Init() {}
    static void ImGui_Term() {}
    static void ImGui_NewFrame() {}
};
//using OxEd_PlatformT = OxEd_Platform_dummy;

#include "OxEd_platform_win32.h"

#endif // OXED_PLATFORM_H

