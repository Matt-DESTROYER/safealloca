# safealloca

A **quick** attempt to make [`alloca`](https://man7.org/linux/man-pages/man3/alloca.3.html?ref=hackernoon.com) safe(er).

This is not by any means a guarantee that this is safe completely (and in fact **it likely is not**).

> This is designed to be somewhat cross-platform, but every compiler/architecture combination will likely require additional preprocessor branches to be supported.
> Feel free to leave an issue if you have an unsupported combination!

## Warning

Obviously `alloca` should be avoided in general, this is purely an experiment to make it safer.

`alloca` can easily be misused, i.e. using it within a loop: each iteration will allocate to the stack, but *will not be deallocated at the end of the loop scope*, resulting in the memory accumulating and potentially causing a stack overflow.

`SAFE_ALLOCA` is *designed* to avoid triggering a stack overflow by refusing allocations when the remaining stack space is too small.
However, this is **not a guarantee** against stack overflow.
For example, if you call it enough times and then have additional variables or stack allocations later in the function, it is still possible to trigger an overflow.

## Usage

This is a very easy-to-use, drop-in library: just copy the `safealloca.h` header file and use as you wish.

`SAFE_ALLOCA` requires its stack bounds to be initialised before use.
By default, this is done explicitly with `SAFE_ALLOCA_INIT()`.
Initialisation is **per-thread**, so each thread that uses `SAFE_ALLOCA` must initialise itself.

The `SAFE_ALLOCA_IMPLEMENTATION` define causes the implementation to be included rather than just the declarations and macros.

```c
// this is required _on Linux_ (the preprocessor will
// tell you if you need this)
#define _GNU_SOURCE

#include <stdio.h>

// define this in exactly one source file
#define SAFE_ALLOCA_IMPLEMENTATION
#include "safealloca.h"

int main(int argc, char* argv[]) {
	// this needs to be run before calling `SAFE_ALLOCA`
	if (!SAFE_ALLOCA_INIT()) {
		return 1;
	}

	size_t letters_count = 3;
	char* letters;

	SAFE_ALLOCA(letters, letters_count + 1);

	if (letters == NULL) {
		return 1;
	}

	for (int i = 0; i < letters_count; i++) {
		letters[i] = 'a' + i;
	}
	letters[letters_count] = '\0';

	printf("%s\n", letters);

	// ...
}
```

If you prefer not to explicitly initialise each thread, see [`SAFE_ALLOCA_LAZY_INITIALISATION`](#safe_alloca_lazy_initialisation).

## Configuration

### `SAFE_ALLOCA_SAFETY_MARGIN`

```c
#define SAFE_ALLOCA_SAFETY_MARGIN (1024 * 64)
```

Specifies the safety margin, in bytes, that `SAFE_ALLOCA` will preserve.

The default is **64 KiB**.

For example:

```c
#define SAFE_ALLOCA_SAFETY_MARGIN 4096
```

If the remaining available stack space is smaller than the configured margin, `SAFE_ALLOCA` will fail the allocation.

### `SAFE_ALLOCA_LAZY_INITIALISATION`

```c
#define SAFE_ALLOCA_LAZY_INITIALISATION
```

Enables lazy initialisation of the stack bounds.

With this enabled, you do not need to explicitly call `SAFE_ALLOCA_INIT()` before using `SAFE_ALLOCA`.

This comes with a **non-negligible performance hit**, since the stack bounds may need to be checked for initialisation during allocation.

By default, explicit initialisation is enabled because it is significantly faster.

## API

### `SAFE_ALLOCA_INIT`

```c
bool SAFE_ALLOCA_INIT(void);
```

Initialises the cached stack bounds for the **current thread**.

This must be called before attempting to use `SAFE_ALLOCA`, unless `SAFE_ALLOCA_LAZY_INITIALISATION` is enabled.

Returns `true` if the stack bounds were successfully determined, or `false` if they could not be determined.

### `SAFE_ALLOCA`

```c
SAFE_ALLOCA(/* void* */ ptr, /* size_t */ size);
```

Attempts to safely perform a stack allocation using `RAW_ALLOCA`.

`ptr` is an output variable which will receive the allocated memory, or `NULL` if the allocation is rejected.

The allocation is rejected if the library cannot determine that there is enough remaining stack space while preserving the configured `SAFE_ALLOCA_SAFETY_MARGIN`.

> **Note:** This is a best-effort check, not a guarantee against stack overflow.

### `RAW_ALLOCA`

```c
RAW_ALLOCA(size);
```

Calls the default `alloca` equivalent supplied by the compiler/OS.

This bypasses the safety checks provided by `SAFE_ALLOCA` and is mainly provided for benchmarking, comparison, and advanced use.

## Benchmark

According to my very quick and potentially very wrong benchmark:

* `alloca` is around **1.5–2× faster** than `malloc`.
* `SAFE_ALLOCA` is about **10% slower** than `alloca`, or around **1.5× faster** than `malloc`.

> Benchmark tested on Linux (Gentoo).

> **Note:** Benchmark order matters!
>
> I chose to run the `SAFE_ALLOCA` benchmark first, which means it runs cold. Switching `SAFE_ALLOCA` and `alloca` in the benchmark brings the difference down to a few hundredths of a second on my device, and implies that `SAFE_ALLOCA`'s performance increases slightly when hot.

These numbers are only intended as a rough indication of the overhead and should not be treated as representative performance across different systems, compilers, or workloads.

## Building the tests yourself

CMake is used to build the tests, although any supported compiler should also work with a command such as:

```sh
cc test.c -o test
```

To build with CMake:

```sh
cd safealloca/
cmake -B build -S src -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cd build
ctest -C Release --verbose
```

These tests are also run on each commit so you can see the performance on a variety of machines and verify that the safe-failure behaviour is working.

## Platform support

Currently supported platforms and their stack-bound discovery mechanisms include:

- **Windows** - uses the Windows thread stack APIs.
- **Linux** - uses pthread thread attributes to obtain the current thread's stack bounds.
- **macOS** - platform-specific support is intended but may require additional implementation.

Compiler/architecture combinations may require additional preprocessor branches, particularly for obtaining the current stack pointer.

If you encounter an unsupported combination, feel free to leave an issue on the GitHub repository.
