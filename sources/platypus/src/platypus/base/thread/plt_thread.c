#include "plt_thread.h"

#include "platypus/base/plt_platform.h"
#include "platypus/base/plt_macros.h"
#include <stdlib.h>

#if PLT_PLATFORM_UNIX
#include <pthread.h>
#elif PLT_PLATFORM_WINDOWS
#include <windows.h>
#endif

typedef struct Plt_Thread {
#if PLT_PLATFORM_UNIX
	pthread_t pthread;
#elif PLT_PLATFORM_WINDOWS
	HANDLE wthread;
#endif
} Plt_Thread;

typedef struct Plt_Thread_Pool_Private_Data Plt_Thread_Pool_Private_Data;
typedef struct Plt_Thread_Pool {
	unsigned int thread_count;
	Plt_Thread **threads;
	void *thread_data;
	void *(*thread_func)(unsigned int thread_id, void *thread_data);
	Plt_Thread_Pool_Private_Data *thread_private_data;

	Plt_Thread_Signal *data_ready_signal;
	
	Plt_Thread_Mutex *completed_thread_mutex;
	unsigned int completed_thread_count;
} Plt_Thread_Pool;

typedef struct Plt_Thread_Signal {
	unsigned int thread_count;

#if PLT_PLATFORM_UNIX
	pthread_mutex_t pmutex;
	pthread_cond_t pcond;
#elif PLT_PLATFORM_WINDOWS
	HANDLE wsemaphore;
#endif
} Plt_Thread_Signal;

typedef struct Plt_Thread_Mutex {
#if PLT_PLATFORM_UNIX
	pthread_mutex_t pmutex;
#elif PLT_PLATFORM_WINDOWS
	HANDLE wmutex;
#endif
} Plt_Thread_Mutex;

Plt_Thread *plt_thread_create(void *(*func)(void *thread_data), void *thread_data) {
	Plt_Thread *thread = malloc(sizeof(Plt_Thread));

#if PLT_PLATFORM_UNIX
	pthread_create(&thread->pthread, NULL, func, thread_data);
#elif PLT_PLATFORM_WINDOWS
	thread->wthread = CreateThread(NULL, 0, func, thread_data, 0, NULL);
#endif

	return thread;
}

void plt_thread_destroy(Plt_Thread **thread) {
#if PLT_PLATFORM_UNIX
	pthread_cancel((*thread)->pthread);
#elif PLT_PLATFORM_WINDOWS
	TerminateThread((*thread)->wthread, 0);
#endif

	free(*thread);
	*thread = NULL;
}

void plt_thread_wait_until_complete(Plt_Thread *thread) {
#if PLT_PLATFORM_UNIX
	pthread_join(thread->pthread, NULL);
#elif PLT_PLATFORM_WINDOWS
	WaitForSingleObject(thread->wthread, INFINITE);
#endif
}

void plt_thread_wait_for_signal(Plt_Thread_Signal *signal) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_lock(&signal->pmutex);
	pthread_cond_wait(&signal->pcond, &signal->pmutex);
	pthread_mutex_unlock(&signal->pmutex);
#elif PLT_PLATFORM_WINDOWS
	WaitForSingleObject(signal->wsemaphore, INFINITE);
#endif
}

typedef struct Plt_Thread_Pool_Private_Data {
	Plt_Thread_Pool *pool;
	unsigned int thread_id;
} Plt_Thread_Pool_Private_Data;

void *_thread_pool_func(void *thread_data) {
	Plt_Thread_Pool_Private_Data *data = thread_data;
	Plt_Thread_Pool *pool = data->pool;
		
	while (true) {
		plt_thread_wait_for_signal(pool->data_ready_signal);

		pool->thread_func(data->thread_id, pool->thread_data);
		
		plt_thread_mutex_lock(pool->completed_thread_mutex);
		pool->completed_thread_count++;
		plt_thread_mutex_unlock(pool->completed_thread_mutex);
	}

	return NULL;
}

Plt_Thread_Pool *plt_thread_pool_create(void *(*func)(unsigned int thread_id, void *thread_data), void *thread_data, unsigned int thread_count) {
	Plt_Thread_Pool *pool = malloc(sizeof(Plt_Thread_Pool));

	pool->thread_count = thread_count;
	pool->thread_data = thread_data;
	
	pool->data_ready_signal = plt_thread_signal_create(thread_count);
	pool->completed_thread_mutex = plt_thread_mutex_create();

	pool->thread_func = func;
	pool->threads = malloc(sizeof(Plt_Thread *) * thread_count);
	pool->thread_private_data = malloc(sizeof(Plt_Thread_Pool_Private_Data) * thread_count);
	for (unsigned int i = 0; i < thread_count; ++i) {
		pool->thread_private_data[i].pool = pool;
		pool->thread_private_data[i].thread_id = i;
		pool->threads[i] = plt_thread_create(_thread_pool_func, pool->thread_private_data + i);
	}

	return pool;
}

void plt_thread_pool_destroy(Plt_Thread_Pool **pool) {
	for (unsigned int i = 0; i < (*pool)->thread_count; ++i) {
		plt_thread_destroy(&(*pool)->threads[i]);
	}
	plt_thread_mutex_destroy(&(*pool)->completed_thread_mutex);
	plt_thread_signal_destroy(&(*pool)->data_ready_signal);

	free((*pool)->threads);
	free((*pool)->thread_private_data);
	free(*pool);
	*pool = NULL;
}

void plt_thread_pool_signal_data_ready(Plt_Thread_Pool *pool) {
	pool->completed_thread_count = 0;
	plt_thread_signal_broadcast(pool->data_ready_signal);
}

void plt_thread_pool_wait_until_complete(Plt_Thread_Pool *pool) {
	while (pool->completed_thread_count < pool->thread_count) {
#if PLT_PLATFORM_UNIX
		usleep(100);
#elif PLT_PLATFORM_WINDOWS
		Sleep(0);
#endif
	}
}

Plt_Thread_Signal *plt_thread_signal_create(unsigned int thread_count) {
	Plt_Thread_Signal *signal = malloc(sizeof(Plt_Thread_Signal));

	signal->thread_count = thread_count;

#if PLT_PLATFORM_UNIX
	pthread_mutex_init(&signal->pmutex, NULL);
	pthread_cond_init(&signal->pcond, NULL);
#elif PLT_PLATFORM_WINDOWS
	signal->wsemaphore = CreateSemaphore(NULL, 0, thread_count, "semaphore");
	plt_assert(signal->wsemaphore, "Failed creating semaphore.\n");
#endif

	return signal;
}

void plt_thread_signal_destroy(Plt_Thread_Signal **signal) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_destroy(&(*signal)->pmutex);
	pthread_cond_destroy(&(*signal)->pcond);
#endif
	free(*signal);
	*signal = NULL;
}

void plt_thread_signal_emit(Plt_Thread_Signal *signal) {
#if PLT_PLATFORM_UNIX
	pthread_cond_signal(&signal->pcond);
#elif PLT_PLATFORM_WINDOWS
	BOOL success = ReleaseSemaphore(signal->wsemaphore, 1, 0);
	plt_assert(success, "Failed releasing semaphore.\n");
#endif
}

void plt_thread_signal_broadcast(Plt_Thread_Signal *signal) {
#if PLT_PLATFORM_UNIX
	pthread_cond_broadcast(&signal->pcond);
#elif PLT_PLATFORM_WINDOWS
	BOOL success = ReleaseSemaphore(signal->wsemaphore, signal->thread_count, 0);
	plt_assert(success, "Failed releasing semaphore.\n");
#endif
}

Plt_Thread_Mutex *plt_thread_mutex_create() {
	Plt_Thread_Mutex *mutex = malloc(sizeof(Plt_Thread_Mutex));

#if PLT_PLATFORM_UNIX
	pthread_mutex_init(&mutex->pmutex, NULL);
#elif PLT_PLATFORM_WINDOWS
	mutex->wmutex = CreateMutex(NULL, FALSE, NULL);
	plt_assert(mutex->wmutex, "Failed creating mutex.\n");
#endif

	return mutex;
}

void plt_thread_mutex_destroy(Plt_Thread_Mutex **mutex) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_destroy(&(*mutex)->pmutex);
#elif PLT_PLATFORM_WINDOWS
	
#endif
	
	free(*mutex);
	*mutex = NULL;
}

void plt_thread_mutex_lock(Plt_Thread_Mutex *mutex) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_lock(&mutex->pmutex);
#elif PLT_PLATFORM_WINDOWS
	WaitForSingleObject(mutex->wmutex, INFINITE);
#endif
}

void plt_thread_mutex_unlock(Plt_Thread_Mutex *mutex) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_unlock(&mutex->pmutex);
#elif PLT_PLATFORM_WINDOWS
	ReleaseMutex(mutex->wmutex);
#endif
}
