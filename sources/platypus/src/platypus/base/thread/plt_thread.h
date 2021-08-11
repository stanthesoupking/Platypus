#pragma once

#include "platypus/platypus.h"

typedef struct Plt_Thread Plt_Thread;
typedef struct Plt_Thread_Pool Plt_Thread_Pool;
typedef struct Plt_Thread_Signal Plt_Thread_Signal;
typedef struct Plt_Thread_Mutex Plt_Thread_Mutex;

// Thread
Plt_Thread *plt_thread_create(void *(*func)(void *thread_data), void *thread_data);
void plt_thread_destroy(Plt_Thread **thread);

void plt_thread_wait_until_complete(Plt_Thread *thread);

void plt_thread_wait_for_signal(Plt_Thread_Signal *signal);

// Thread pool
Plt_Thread_Pool *plt_thread_pool_create(void *(*func)(unsigned int thread_id, void *thread_data), void *thread_data, unsigned int thread_count);
void plt_thread_pool_destroy(Plt_Thread_Pool **pool);

void plt_thread_pool_signal_data_ready(Plt_Thread_Pool *pool);
void plt_thread_pool_wait_until_complete(Plt_Thread_Pool *pool);

// Signal
Plt_Thread_Signal *plt_thread_signal_create();
void plt_thread_signal_destroy(Plt_Thread_Signal **signal);

void plt_thread_signal_emit(Plt_Thread_Signal *signal);
void plt_thread_signal_broadcast(Plt_Thread_Signal *signal);

// Mutex
Plt_Thread_Mutex *plt_thread_mutex_create();
void plt_thread_mutex_destroy(Plt_Thread_Mutex **mutex);

void plt_thread_mutex_lock(Plt_Thread_Mutex *mutex);
void plt_thread_mutex_unlock(Plt_Thread_Mutex *mutex);