# safealloca
A quick attempt to make [`alloca`](https://man7.org/linux/man-pages/man3/alloca.3.html?ref=hackernoon.com) safe.
This is not by any means a guarantee that this is safe, and as seen in the benchmark, this is less performant than malloc, so I can't imagine any usecase where this is a good idea.

According to the very quick and potentially very wrong benchmark:
`alloca` is ~1.5x faster than `malloc`
`SAFE_ALLOCA` is ~25x slower than `malloc`

