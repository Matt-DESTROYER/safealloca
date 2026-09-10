# safealloca
A quick attempt to make [`alloca`](https://man7.org/linux/man-pages/man3/alloca.3.html?ref=hackernoon.com) safe.
This is not by any means a guarantee that this is safe completely.

## Warning
Obviously `alloca` should be avoided in general, this is purely an experiment to make it safer. `alloca` can easily be misused, ie using it within a loop (within which each iteration will allocate to the stack, but _will not be deallocated at the end of the loop scope_)

According to my very quick and potentially very wrong benchmark:
`alloca` is around 1.5 to 2 times faster than `malloc`
`SAFE_ALLOCA` is about 10% slower than `alloca`, or closer to 1.5 times faster than `malloc`
> Benchmark tested on Linux (Gentoo)

> Note on the benchmark, order matters!
> I choose to run the `SAFEALLOCA` bench first as this allows it to run cold. Switching `SAFEALLOCA` and `alloca` in the benchmark brings the difference down to a few hundredths of a second (on my device) and implies `SAFEALLOCA`s performance increases slightly when hot.

