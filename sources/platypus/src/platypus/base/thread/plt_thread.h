#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Thread Plt_Thread;
typedef struct Plt_Thread_Signal Plt_Thread_Signal;
typedef struct Plt_Thread_Mutex Plt_Thread_Mutex;

Plt_Thread *plt_thread_create(void *(*func)(void *thread_data), void *thread_data);
void plt_thread_destroy(Plt_Thread **thread);

void plt_thread_wait_until_complete(Plt_Thread *thread);

void plt_thread_wait_for_signal(Plt_Thread_Signal *signal);

Plt_Thread_Signal *plt_thread_signal_create();
void plt_thread_signal_destroy(Plt_Thread_Signal **signal);

void plt_thread_signal_emit(Plt_Thread_Signal *signal);
void plt_thread_signal_broadcast(Plt_Thread_Signal *signal);

Plt_Thread_Mutex *plt_thread_mutex_create();
void plt_thread_mutex_destroy(Plt_Thread_Mutex **mutex);

void plt_thread_mutex_lock(Plt_Thread_Mutex *mutex);
void plt_thread_mutex_unlock(Plt_Thread_Mutex *mutex);
