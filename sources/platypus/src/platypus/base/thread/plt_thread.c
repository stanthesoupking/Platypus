#include "plt_thread.h"

#include "platypus/base/platform.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct Plt_Thread {
#if PLT_PLATFORM_UNIX
	pthread_t pthread;
#endif
} Plt_Thread;

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