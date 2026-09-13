#ifndef SAFE_ALLOCA_H
#define SAFE_ALLOCA_H

#ifdef _MSC_VER
	#define SAFE_ALLOCA_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
	#define SAFE_ALLOCA_NOINLINE __attribute__((noinline))
#else
	#define SAFE_ALLOCA_NOINLINE
#endif

#ifdef _WIN32
	#include <windows.h>
	#include <processthreadsapi.h>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#ifdef __linux__
		#ifndef _GNU_SOURCE
			#error "Please #define _GNU_SOURCE before including this library or pthreads."
		#endif
	#endif

	#include <pthread.h>
#else
	#error "Unsupported OS: feel free to leave an issue on the GitHub repository!"
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define RAW_ALLOCA __builtin_alloca
	#define GET_CURRENT_STACK_POINTER() (uintptr_t)__builtin_frame_address(0)
#elif defined(_MSC_VER)
	#include <malloc.h>
	#include <intrin.h>
	#define RAW_ALLOCA _alloca
	/* close enough that it probably won't matter (with the default safety margin) */
	#define GET_CURRENT_STACK_POINTER() (uintptr_t)_AddressOfReturnAddress()
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

/* C11+ */
#ifdef __STDC_VERSION__
	#if __STDC_VERSION__ >= 201112L
		#define SAFE_ALLOCA_THREAD_LOCAL _Thread_local
	#else
		#error "C11+ is required for thread-safety, feel free to ignore if you don't require this!"
	#endif
#elif defined(_MSC_VER)
	#define SAFE_ALLOCA_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
	#define SAFE_ALLOCA_THREAD_LOCAL __thread
#else
	#error "Thread-local storage is required for thread-safety, this compiler is not yet supported or incompatible. If you don't need thread-safety, feel free to remove this line!"
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SAFE_ALLOCA_SAFETY_MARGIN
	#define SAFE_ALLOCA_SAFETY_MARGIN (1024 * 64) /* 64 KiB */
#endif

#ifndef SAFE_ALLOCA_LAZY_INITIALISATION
	#define SAFE_ALLOCA_EXPLICIT_INITIALISATION
#endif

typedef struct safe_alloca_stack {
	uintptr_t low;
	uintptr_t high;
} safe_alloca_stack_t;

bool get_stack_limit(safe_alloca_stack_t* stack);
SAFE_ALLOCA_NOINLINE size_t get_stack_available(void);
bool SAFE_ALLOCA_INIT(void);

#ifdef SAFE_ALLOCA_IMPLEMENTATION

static SAFE_ALLOCA_THREAD_LOCAL safe_alloca_stack_t stack_limit;

bool get_stack_limit(safe_alloca_stack_t* stack) {
#ifdef _WIN32
	ULONG_PTR low;
	ULONG_PTR high;
	GetCurrentThreadStackLimits(&low, &high);

	stack->low = (uintptr_t)low;
	stack->high = (uintptr_t)high;
#else
	void* stack_addr;
	size_t stack_size;

#ifdef __APPLE__
	stack_addr = pthread_get_stackaddr_np(pthread_self());
	stack_size = pthread_get_stacksize_np(pthread_self());

	stack->high = (uintptr_t)stack_addr;
	if (stack_size > stack->high)
		return false;

	stack->low = stack->high - (uintptr_t)stack_size;
#else
	pthread_attr_t attr;

	if (pthread_getattr_np(pthread_self(), &attr) != 0)
		return false;

	if (pthread_attr_getstack(&attr, &stack_addr, &stack_size) != 0) {
		pthread_attr_destroy(&attr);
		return false;
	}
	pthread_attr_destroy(&attr);

	stack->low = (uintptr_t)stack_addr;
	if (stack_size > UINTPTR_MAX - stack->low)
		return false;

	stack->high = stack->low + (uintptr_t)stack_size;
#endif
#endif

	return true;

	/* stack size      = stack.high - stack.low */
	/* stack remaining = stack pointer - stack.low */
}

SAFE_ALLOCA_NOINLINE size_t get_stack_available(void) {
#ifdef SAFE_ALLOCA_LAZY_INITIALISATION
	if (stack_limit.low == 0 && !get_stack_limit(&stack_limit))
		return 0;
#endif

	uintptr_t stack_pointer = GET_CURRENT_STACK_POINTER();

	/* NOTE: we assume stack grows downwards */

	if (stack_limit.low >= stack_limit.high
			|| stack_pointer < stack_limit.low
			|| stack_pointer > stack_limit.high)
		return 0;

	return (size_t)(stack_pointer - stack_limit.low);
}

#endif

bool SAFE_ALLOCA_INIT(void) {
	return get_stack_limit(&stack_limit);
}

#define SAFE_ALLOCA(ptr, size) \
	size_t requested_size = size; \
	size_t available = get_stack_available(); \
	if (available > SAFE_ALLOCA_SAFETY_MARGIN \
			&& requested_size <= available - SAFE_ALLOCA_SAFETY_MARGIN) \
		*ptr = RAW_ALLOCA(requested_size); \
	else \
		*ptr = NULL

#endif
