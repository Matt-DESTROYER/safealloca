# safealloca
A quick attempt to make [`alloca`](https://man7.org/linux/man-pages/man3/alloca.3.html?ref=hackernoon.com) safe.
This is not by any means a guarantee that this is safe completely.

> This is somewhat cross-platform, but every compiler/architecture combination will likely require new preprocessor branches to be supported. Feel free to leave an issue if you have an unsupported combination!

## Warning
Obviously `alloca` should be avoided in general, this is purely an experiment to make it safer. `alloca` can easily be misused, ie using it within a loop: each iteration will allocate to the stack, but _will not be deallocated at the end of the loop scope_, resulting in the memory accumulating and potential stack overflows (`SAFE_ALLOCA` itself should not trigger a stack overflow, however if you call it enough and then you have some additional variables defined later, it is still possible).

## Usage
This is a very easy to use, drop-in library, just copy the `safealloca.h` header file and use as you wish.

In order for `SAFE_ALLOCA` to work, it has an initialisation function that must be run at the start of the program, and a `#define` flag triggers the actual implementation rather than just including the definitions.

```c
#include <stdio.h>

// if you just want to include the definitions/macros,
// you don't need to define this
#define SAFE_ALLOCA_IMPLEMENTATION
#include "safealloca.h"

int main(int argc, char* argv[]) {
	// this needs to be run at the start of your program
	// it grabs a reference to the bottom of your stack
	// and caches the available stack size
	INIT_SAFE_ALLOCA();

	// ...

	size_t letters_count = 3;
	char* letters = (char*)SAFE_ALLOCA(letters_count + 1);
	for (int i = 0; i < letters_count; i++) {
		letters[i] = 'a' + i;
	}
	letters[letters_count] = '\0';

	printf("%s\n", letters);

	// ...
}
```

You can also specify a safety margin, by default this is 4kb (4096 bytes).
If the remaining available stack memory is smaller than this, `SAFE_ALLOCA` will simply fail and return NULL.
This can be specified by defining `SAFE_ALLOCA_SAFETY_MARGIN` (the margin you want in **bytes**).
```c
// this is the default margin
#define SAFE_ALLOCA_SAFETY_MARGIN 4096
```

## Benchmark
According to my very quick and potentially very wrong benchmark:
`alloca` is around 1.5 to 2 times faster than `malloc`
`SAFE_ALLOCA` is about 10% slower than `alloca`, or closer to 1.5 times faster than `malloc`
> Benchmark tested on Linux (Gentoo)

> Note on the benchmark, order matters!
> I choose to run the `SAFE_ALLOCA` bench first as this allows it to run cold. Switching `SAFE_ALLOCA` and `alloca` in the benchmark brings the difference down to a few hundredths of a second (on my device) and implies `SAFE_ALLOCA`s performance increases slightly when hot.
