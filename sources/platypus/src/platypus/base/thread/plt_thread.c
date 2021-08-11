#include "plt_thread.h"

#include "platypus/base/platform.h"
#include <stdlib.h>

#if PLT_PLATFORM_UNIX
#include <pthread.h>
#endif

typedef struct Plt_Thread {
#if PLT_PLATFORM_UNIX
	pthread_t pthread;
#endif
} Plt_Thread;

typedef struct Plt_Thread_Signal {
#if PLT_PLATFORM_UNIX
	pthread_mutex_t pmutex;
	pthread_cond_t pcond;
#endif
} Plt_Thread_Signal;

typedef struct Plt_Thread_Mutex {
#if PLT_PLATFORM_UNIX
	pthread_mutex_t pmutex;
#endif
} Plt_Thread_Mutex;

Plt_Thread *plt_thread_create(void *(*func)(void *thread_data), void *thread_data) {
	Plt_Thread *thread = malloc(sizeof(Plt_Thread));

#if PLT_PLATFORM_UNIX
	pthread_create(&thread->pthread, NULL, func, thread_data);
#endif

	return thread;
}

void plt_thread_destroy(Plt_Thread **thread) {
#if PLT_PLATFORM_UNIX
	pthread_cancel((*thread)->pthread);
#endif

	free(*thread);
	*thread = NULL;
}

void plt_thread_wait_until_complete(Plt_Thread *thread) {
#if PLT_PLATFORM_UNIX
	pthread_join(thread->pthread, NULL);
#endif
}

void plt_thread_wait_for_signal(Plt_Thread_Signal *signal) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_lock(&signal->pmutex);
	pthread_cond_wait(&signal->pcond, &signal->pmutex);
	pthread_mutex_unlock(&signal->pmutex);
#endif
}

Plt_Thread_Signal *plt_thread_signal_create() {
	Plt_Thread_Signal *signal = malloc(sizeof(Plt_Thread_Signal));

#if PLT_PLATFORM_UNIX
	pthread_mutex_init(&signal->pmutex, NULL);
	pthread_cond_init(&signal->pcond, NULL);
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
#endif
}

void plt_thread_signal_broadcast(Plt_Thread_Signal *signal) {
#if PLT_PLATFORM_UNIX
	pthread_cond_broadcast(&signal->pcond);
#endif
}

Plt_Thread_Mutex *plt_thread_mutex_create() {
	Plt_Thread_Mutex *mutex = malloc(sizeof(Plt_Thread_Mutex));

#if PLT_PLATFORM_UNIX
	pthread_mutex_init(&mutex->pmutex, NULL);
#endif

	return mutex;
}

void plt_thread_mutex_destroy(Plt_Thread_Mutex **mutex) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_destroy(&(*mutex)->pmutex);
#endif
	
	free(*mutex);
	*mutex = NULL;
}

void plt_thread_mutex_lock(Plt_Thread_Mutex *mutex) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_lock(&mutex->pmutex);
#endif
}

void plt_thread_mutex_unlock(Plt_Thread_Mutex *mutex) {
#if PLT_PLATFORM_UNIX
	pthread_mutex_unlock(&mutex->pmutex);
#endif
}
