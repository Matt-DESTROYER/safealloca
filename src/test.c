#include <stdlib.h>
#include <stdio.h>

#define SAFE_ALLOCA_IMPLEMENTATION
#include "safealloca.h"

#ifdef _MSC_VER
	#define NOINLINE __declspec(noinline)
#else
	#define NOINLINE __attribute__((noinline))
#endif

#ifdef _WIN32
#include <windows.h>
typedef LARGE_INTEGER benchmark_time_t;

static void get_current_time(benchmark_time_t* t) {
	QueryPerformanceCounter(t);
}
static double get_time_diff(benchmark_time_t start, benchmark_time_t end) {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
}
#else
#include <time.h>
typedef struct timespec benchmark_time_t;

static void get_current_time(benchmark_time_t* t) {
	clock_gettime(CLOCK_MONOTONIC, t);
}

static double get_time_diff(benchmark_time_t start, benchmark_time_t end) {
	return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}
#endif

// some voodoo to tell the compiler not to optimise away redundant memory allocations
// https://stackoverflow.com/questions/40122141/preventing-compiler-optimizations-while-benchmarking
#ifdef _MSC_VER
#pragma optimize("", off)
static void escape(void* p) {
	(void)p;
}
#pragma optimize("", on)
#else
static void escape(void* p) {
	__asm__ volatile("" : : "g"(p) : "memory");
}
#endif

NOINLINE void run_unsafe_alloca(size_t size) {
	char* buffer = (char*)alloca(size);
	if (buffer == NULL)
		return;

	buffer[0] = 'A';
	escape(buffer);
}

NOINLINE void run_safe_alloca(size_t size) {
	char* buffer = (char*)SAFE_ALLOCA(size);
	if (buffer == NULL)
		return;

	buffer[0] = 'A';
	escape(buffer);
}

NOINLINE void run_malloc(size_t size) {
	char* buffer = (char*)malloc(size);
	if (buffer == NULL)
		return;

	buffer[0] = 'A';
	escape(buffer);
	free(buffer);
}

NOINLINE void test_safe_alloca_exhaustion() {
	printf("Starting SAFE_ALLOCA exhaustion test...\n");

	size_t chunk_size = 64 * 1024;
	size_t count = 0;

	while (1) {
		char* buffer = (char*)SAFE_ALLOCA(chunk_size);

		if (buffer == NULL) {
			printf("SUCCESS: `SAFE_ALLOCA` returned NULL and prevented a stack overflow!\n");
			printf("Stopped safely after allocating %zu chunks (%zu bytes).\n", count, count * chunk_size);
			break;
		}

		buffer[0] = 'X';
		escape(buffer);

		count++;
	}
}

void run_benchmark(size_t iterations, size_t allocation_size) {
	benchmark_time_t start, end;

	// Warmup
	printf("Warming up the CPU...\n");
	volatile size_t dummy = 0;
	for (size_t i = 0; i < 100000000; i++) {
		dummy += i;
	}

	printf("Benchmarking %zu iterations of allocating %zu bytes...\n", iterations, allocation_size);

	// SAFE_ALLOCA
	get_current_time(&start);

	for (size_t i = 0; i < iterations; i++) {
		run_safe_alloca(allocation_size);
	}

	get_current_time(&end);
	double safe_alloca_time = get_time_diff(start, end);

	printf("SAFEALLOCA time: %f seconds\n", safe_alloca_time);

	// alloca
	get_current_time(&start);

	for (size_t i = 0; i < iterations; i++) {
		run_unsafe_alloca(allocation_size);
	}

	get_current_time(&end);
	double unsafe_alloca_time = get_time_diff(start, end);

	printf("alloca time: %f seconds\n", unsafe_alloca_time);

	// malloc
	get_current_time(&start);

	for (size_t i = 0; i < iterations; i++) {
		run_malloc(allocation_size);
	}

	get_current_time(&end);
	double malloc_time = get_time_diff(start, end);

	printf("malloc/free time: %f seconds\n", malloc_time);

	if (safe_alloca_time > malloc_time) {
		printf("\n[WARNING] `SAFE_ALLOCA` was slower than malloc (%f vs %f)\n", safe_alloca_time, malloc_time);
	} else {
		printf("\n[SUCCESS] `SAFE_ALLOCA` outperformed malloc.\n");
	}
}

int main() {
	INIT_SAFE_ALLOCA();

	test_safe_alloca_exhaustion();

	run_benchmark(100000000, 256);

	return EXIT_SUCCESS;
}

