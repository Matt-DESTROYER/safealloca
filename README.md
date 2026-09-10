# safealloca
A quick attempt to make [`alloca`](https://man7.org/linux/man-pages/man3/alloca.3.html?ref=hackernoon.com) safe.
This is not by any means a guarantee that this is safe, and as seen in the benchmark.

According to my very quick and potentially very wrong benchmark:
`alloca` is around 1.5 to 2 times faster than `malloc`
`SAFE_ALLOCA` is about 10% slower than `alloca`, or closer to 1.5 times faster than `malloc`

> Note on the benchmark, order matters!
> I choose to run the `SAFEALLOCA` bench first as this is allows it to run cold. Switching `SAFEALLOCA` and `alloca` in the benchmark brings the difference down to a few hundredths of a second (on my device).

> Tested on Gentoo Linux

