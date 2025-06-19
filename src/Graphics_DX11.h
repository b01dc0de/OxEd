#ifndef GRAPHICS_DX11_H
#define GRAPHICS_DX11_H

#include "OxEd.h"

struct Graphics_DX11
{
    static int Init();
    static void Term();
    static void UpdateAndDraw();
};

#endif // GRAPHICS_DX11_H

