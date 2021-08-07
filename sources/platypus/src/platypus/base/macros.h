#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

#if PLT_PLATFORM_WINDOWS
#include <time.h>
#elif PLT_PLATFORM_MACOS
#include <unistd.h>
#include <sys/time.h>
#elif PLT_PLATFORM_LINUX
#include <unistd.h>
#include <time.h>
#endif

#define plt_cast(type, var) *((type*)&var)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define plt_assert(cond, message) \
if(!(cond)) { \
	printf("------------------------------------------------------------\n"); \
	printf("  Assertion failed on line %d of %s:  \n\t %s\n", __LINE__, __FILENAME__, message);\
	printf("------------------------------------------------------------\n"); \
	abort();\
}

#define plt_abort(message) \
printf("------------------------------------------------------------\n"); \
printf("  Abort on line %d of %s:  \n\t %s\n", __LINE__, __FILENAME__, message);\
printf("------------------------------------------------------------\n"); \
abort();

static inline float plt_get_millis() {
#if PLT_PLATFORM_WINDOWS
	clock_t c = clock();
	float elapsed = ((float)c) / CLOCKS_PER_SEC * 1000.0f;
	return elapsed;
#elif PLT_PLATFORM_MACOS 
	struct timespec curTime;
	clock_gettime(_CLOCK_REALTIME, &curTime);
	return curTime.tv_nsec / 1000000.0f;
#elif PLT_PLATFORM_LINUX
	struct timespec curTime;
	clock_gettime(CLOCK_REALTIME, &curTime);
	return curTime.tv_nsec / 1000000.0f;
#endif
}

static int _plt_timer_depth = 0;

#define plt_timer_start(timer_name) \
_plt_timer_depth++; \
float timer_name = plt_get_millis();

#define plt_timer_end(timer_name, string_name) \
_plt_timer_depth--; \
for (int i = 0; i < _plt_timer_depth; ++i) { printf("  "); } \
if (_plt_timer_depth > 0) { printf("- "); }\
printf("%s completed in %.2fms\n", string_name, plt_get_millis() - timer_name);
