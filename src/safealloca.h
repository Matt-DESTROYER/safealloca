#ifndef SAFE_ALLOCA_H
#define SAFE_ALLOCA_H

#include <stddef.h>
#include <alloca.h>
#include <stdint.h>
#include <sys/resource.h>

#define SAFE_ALLOCA_SAFETY_MARGIN 4096

extern void* stack_base;

size_t get_stack_available(void);

#define INIT_SAFE_ALLOCA() \
do { \
	int temp = 0; \
	stack_base = &temp; \
} while(0);

#define SAFE_ALLOCA(size) ((size + SAFE_ALLOCA_SAFETY_MARGIN) < get_stack_available() ? alloca(size) : NULL)

#ifdef SAFE_ALLOCA_IMPLEMENTATION

void* stack_base = NULL;

size_t get_stack_available(void) {
		struct rlimit rlim;

		if (getrlimit(RLIMIT_STACK, &rlim) != 0)
				return 0;

		if (rlim.rlim_cur == RLIM_INFINITY)
				return SIZE_MAX;

		int temp = 0;
		void* stack_pointer = (void*)&temp;

		// note: stack grows downwards
		size_t stack_used = (size_t)stack_base - (size_t)stack_pointer;

		if (stack_used >= rlim.rlim_cur)
				return 0;

		return rlim.rlim_cur - stack_used;
}

#endif
#endif

