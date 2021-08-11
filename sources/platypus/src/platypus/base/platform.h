#pragma once

#ifdef _MSC_VER
#define PLT_PLATFORM_WINDOWS 1
#elif __APPLE__
#define PLT_PLATFORM_MACOS 1
#define PLT_PLATFORM_UNIX 1
#elif __unix__
#define PLT_PLATFORM_LINUX 1
#define PLT_PLATFORM_UNIX 1
#else
#define PLT_PLATFORM_UNKNOWN 1
#endif

#if PLT_PLATFORM_UNIX
#include <unistd.h>
#elif PLT_PLATFORM_WINDOWS
#include <windows.h>
#endif

const static unsigned int plt_platform_get_core_count() {
#if PLT_PLATFORM_UNIX
	return sysconf(_SC_NPROCESSORS_ONLN);
#elif PLT_PLATFORM_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#else
	// Unsupported platform, return 1 by default.
	return 1;
#endif
}