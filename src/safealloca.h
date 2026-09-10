#ifndef SAFE_ALLOCA_H
#define SAFE_ALLOCA_H

#ifdef _WIN32
	#include <windows.h>
	#include <processthreadsapi.h>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <sys/resource.h>
#else
	#error "Unsupported OS: feel free to leave an issue on the GitHub repository!"
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define RAW_ALLOCA __builtin_alloca
	#define GET_CURRENT_STACK_POINTER() __builtin_frame_address(0)
#elif defined(_MSC_VER)
	#include <malloc.h>
	#include <intrin.h>
	#define RAW_ALLOCA _alloca
	// close enough that it probably won't matter (with the default safety margin)
	#define GET_CURRENT_STACK_POINTER() (size_t)_AddressOfReturnAddress()
#else
	#if defined(__has_include)
		#if __has_include(<alloca.h>)
			#include <alloca.h>
			#define RAW_ALLOCA alloca
		#endif
	#endif

	#ifdef RAW_ALLOCA
		#error "Unsupported compiler: `alloca` was located, but stack pointer intrinsics are missing. Feel free to leave an issue on the GitHub repository!"
	#else
		#error "Unsupported compiler: feel free to leave an issue on the GitHub repository!"
	#endif
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef SAFE_ALLOCA_SAFETY_MARGIN
#define SAFE_ALLOCA_SAFETY_MARGIN 4096
#endif

extern void* stack_base;
extern size_t stack_limit;

size_t get_stack_limit(void);
size_t get_stack_available(void);

#ifdef SAFE_ALLOCA_IMPLEMENTATION

void* stack_base = NULL;
size_t stack_limit = 0;

size_t get_stack_limit(void) {
#ifdef _WIN32
	ULONG_PTR low_limit, high_limit;
	GetCurrentThreadStackLimits(&low_limit, &high_limit);
	return (size_t)(high_limit - low_limit);
#else
	struct rlimit rlim;

	if (getrlimit(RLIMIT_STACK, &rlim) != 0)
		return 0;

	if (rlim.rlim_cur == RLIM_INFINITY)
		return SIZE_MAX;

	return rlim.rlim_cur;
#endif
}

size_t get_stack_available(void) {
	void* stack_pointer = GET_CURRENT_STACK_POINTER();

	// note: stack grows downwards
	size_t stack_used = (size_t)stack_base - (size_t)stack_pointer;

	if (stack_used >= stack_limit)
		return 0;

	return stack_limit - stack_used;
}

#endif

#define INIT_SAFE_ALLOCA() \
	stack_base = GET_CURRENT_STACK_POINTER(); \
	stack_limit = get_stack_limit();

#define SAFE_ALLOCA(size) (((size) + SAFE_ALLOCA_SAFETY_MARGIN) < get_stack_available() ? RAW_ALLOCA(size) : NULL)

#endif
