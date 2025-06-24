#ifndef OXED_GFX_H
#define OXED_GFX_H

struct OxEd_Gfx_dummy
{
    static const char* Name() { return "GfxDummy"; }
    static void FrameBegin() {}
    static void FrameEnd() {}
    static bool Init() { return true; }
    static bool Term() { return true; }
    static void ImGui_Init() {}
    static void ImGui_Term() {}
    static void ImGui_NewFrame() {}
    static void ImGui_RenderDrawData(ImDrawData*) {}
};

//using OxEd_GfxT = OxEd_Gfx_dummy;

#include "OxEd_gfx_dx11.h"

#endif // OXED_GFX_H

