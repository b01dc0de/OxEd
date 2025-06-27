#ifndef OXED_H
#define OXED_H

// C++ standard libary:
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Dear ImGui:
#include "imgui/imgui.h"

// Fixed width types:
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

// Build configuration:
#include "OxEd_build.h"
// Project headers:
#include "Math.h"
#include "Utils.h"
// OxEd platform:
#include "OxEd_platform.h"
// OxEd gfx backend:
#include "OxEd_gfx.h"

// Globals
extern bool bRunning;
extern UINT WinResX;
extern UINT WinResY;

#endif // OXED_H

