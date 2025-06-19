#ifndef OXED_WIN32_H
#define OXED_WIN32_H

//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#if _DEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif // _DEBUG

#endif // OXED_WIN32_H

