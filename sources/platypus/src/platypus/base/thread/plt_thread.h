#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Thread Plt_Thread;

Plt_Thread *plt_thread_create(void *(*func)(void *thread_data), void *thread_data);
void plt_thread_destroy(Plt_Thread **thread);

void plt_thread_wait_until_complete(Plt_Thread *thread);