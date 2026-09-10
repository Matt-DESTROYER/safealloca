#ifndef SAFE_ALLOCA_H
#define SAFE_ALLOCA_H

#include <stddef.h>
#include <alloca.h>
#include <stdint.h>
#include <sys/resource.h>

#ifndef SAFE_ALLOCA_SAFETY_MARGIN
#define SAFE_ALLOCA_SAFETY_MARGIN 4096
#endif

extern void* stack_base;
extern size_t stack_limit;

size_t get_stack_limit(void);
static size_t get_stack_available(void);

#ifdef SAFE_ALLOCA_IMPLEMENTATION

void* stack_base = NULL;
size_t stack_limit = 0;

size_t get_stack_limit(void) {
	struct rlimit rlim;

	if (getrlimit(RLIMIT_STACK, &rlim) != 0)
		return 0;

	if (rlim.rlim_cur == RLIM_INFINITY)
		return SIZE_MAX;

	return rlim.rlim_cur;
}

static size_t get_stack_available(void) {
	void* stack_pointer = __builtin_frame_address(0);

	// note: stack grows downwards
	size_t stack_used = (size_t)stack_base - (size_t)stack_pointer;

	if (stack_used >= stack_limit)
		return 0;

	return stack_limit - stack_used;
}

#endif

#define INIT_SAFE_ALLOCA() \
	stack_base = __builtin_frame_address(0); \
	stack_limit = get_stack_limit();

#define SAFE_ALLOCA(size) ((size + SAFE_ALLOCA_SAFETY_MARGIN) < get_stack_available() ? alloca(size) : NULL)

#endif

