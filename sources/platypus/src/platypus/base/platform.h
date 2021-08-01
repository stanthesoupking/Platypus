#pragma once

#ifdef _MSC_VER
#define PLT_PLATFORM_WINDOWS 1
#elif __APPLE__
#define PLT_PLATFORM_MACOS 1
#else
#define PLT_PLATFORM_UNKNOWN 1
#endif