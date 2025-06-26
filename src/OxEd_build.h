#ifndef OXED_BUILD_H
#define OXED_BUILD_H

// TODO: Actually detect platform/gfx/config correctly

#define OXED_PLATFORM_WINDOWS() (1)
#define OXED_PLATFORM_OTHER() (!OXED_PLATFORM_WINDOWS())

#define OXED_GFXBACKEND_DX11() (1)
#define OXED_GFXBACKEND_OTHER() (!OXED_GFXBACKED_OTHER())

#define OXED_CONFIG_DEBUG() (_DEBUG)
#define OXED_CONFIG_RELEASE() (!OXED_CONFIG_DEBUG())

#endif // OXED_BUILD_H
