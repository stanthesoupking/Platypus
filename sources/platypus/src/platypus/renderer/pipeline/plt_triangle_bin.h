#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Triangle_Bin {
	unsigned int triangle_count;

	unsigned int *triangle_index;
	Plt_Triangle_Processor_Result *tp_result;
} Plt_Triangle_Bin;