#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SAFE_ALLOCA_IMPLEMENTATION
#include "safealloca.h"

// some voodoo to tell the compiler not to optimise away redundant memory allocations
// https://stackoverflow.com/questions/40122141/preventing-compiler-optimizations-while-benchmarking
static void escape(void *p) {
	__asm__ volatile("" : : "g"(p) : "memory");
}

__attribute__((noinline)) void run_unsafe_alloca(size_t size) {
	char* buffer = (char*)alloca(size);
	if (buffer == NULL)
		return;

	buffer[0] = 'A';
	escape(buffer);
}

__attribute__((noinline)) void run_safe_alloca(size_t size) {
	char* buffer = (char*)SAFE_ALLOCA(size);
	if (buffer == NULL)
		return;

	buffer[0] = 'A';
	escape(buffer);
}

__attribute__((noinline)) void run_malloc(size_t size) {
	char* buffer = (char*)malloc(size);
	if (buffer == NULL)
		return;

	buffer[0] = 'A';
	escape(buffer);
	free(buffer);
}

double get_time_diff(struct timespec start, struct timespec end) {
	return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void run_benchmark(size_t iterations, size_t allocation_size) {
	struct timespec start, end;

	// Warmup
	printf("Warming up the CPU...\n");
	volatile int dummy = 0;
	for (size_t i = 0; i < 100000000; i++) {
		dummy += i;
	}

	printf("Benchmarking %zu iterations of allocating %zu bytes...\n", iterations, allocation_size);

	// SAFE_ALLOCA
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (size_t i = 0; i < iterations; i++) {
		run_safe_alloca(allocation_size);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	double safe_alloca_time = get_time_diff(start, end);

	printf("SAFEALLOCA time: %f seconds\n", safe_alloca_time);

	// alloca
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (size_t i = 0; i < iterations; i++) {
		run_unsafe_alloca(allocation_size);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	double unsafe_alloca_time = get_time_diff(start, end);

	printf("alloca time: %f seconds\n", unsafe_alloca_time);

	// malloc
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (size_t i = 0; i < iterations; i++) {
		run_malloc(allocation_size);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	double malloc_time = get_time_diff(start, end);

	printf("malloc/free time: %f seconds\n", malloc_time);
}

int main() {
	INIT_SAFE_ALLOCA();

	run_benchmark(100000000, 256);

	return EXIT_SUCCESS;
}

