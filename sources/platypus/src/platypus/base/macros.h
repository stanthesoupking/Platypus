#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define plt_min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define plt_max(X, Y) (((X) > (Y)) ? (X) : (Y))

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