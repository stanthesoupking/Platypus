#pragma once

#include "platypus/platypus.h"

void plt_object_path_get_component(char *dest_component, const char *src_path, unsigned int *component_length, bool *is_last_component, bool *is_go_to_parent);
