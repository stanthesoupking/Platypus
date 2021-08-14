#pragma once

#include "plt_thread.h"
#include <stdlib.h>

typedef struct Plt_Thread_Safe_Stack {
	unsigned int *values;
	unsigned int count;
	unsigned int capacity;

	Plt_Thread_Mutex *mutex;
} Plt_Thread_Safe_Stack;

static inline Plt_Thread_Safe_Stack *plt_thread_safe_stack_create(unsigned int capacity) {
	Plt_Thread_Safe_Stack *stack = malloc(sizeof(Plt_Thread_Safe_Stack));
	stack->values = malloc(sizeof(unsigned int) * capacity);
	stack->capacity = capacity;
	stack->count = 0;
	stack->mutex = plt_thread_mutex_create();
	return stack;
}

static inline void plt_thread_safe_stack_destroy(Plt_Thread_Safe_Stack **stack) {
	plt_thread_mutex_destroy(&(*stack)->mutex);
	free((*stack)->values);
	free(*stack);
	*stack = NULL;
}

static inline void plt_thread_safe_stack_lock(Plt_Thread_Safe_Stack *stack) {
	plt_thread_mutex_lock(stack->mutex);
}

static inline void plt_thread_safe_stack_unlock(Plt_Thread_Safe_Stack *stack) {
	plt_thread_mutex_unlock(stack->mutex);
}

static inline void plt_thread_safe_stack_push(Plt_Thread_Safe_Stack *stack, unsigned int v) {
	stack->values[stack->count++] = v;
}

static inline bool plt_thread_safe_stack_pop(Plt_Thread_Safe_Stack *stack, unsigned int *v) {
	plt_thread_safe_stack_lock(stack);
	int i = stack->count--;
	plt_thread_safe_stack_unlock(stack);
	if (i < 1) {
		return false;
	} else {
		*v = stack->values[i - 1];
		return true;
	}
}

static inline unsigned int plt_thread_safe_stack_get_count(Plt_Thread_Safe_Stack *stack) {
	return stack->count;
}
