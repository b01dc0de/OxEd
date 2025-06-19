#ifndef OXED_H
#define OXED_H

// C++ standard libary:
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
// DX11:
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

// Dear ImGui:
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

// Types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using f32 = float;
using f64 = float;

// Globals
extern bool bRunning;
extern HWND hWindow;
extern UINT WinResX;
extern UINT WinResY;

#define APPNAME() ("OxEd")
#define ASSERT(Exp) if (!(Exp)) { DebugBreak(); }

// Project headers
#include "Math.h"
#include "Utils.h"

#endif // OXED_H

