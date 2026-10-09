# Performance benchmarks

The performance executable measures the steady-state cost of transform construction,
DWT, IDWT, boundary extension, and an eight-level low-pass filter. It reports elapsed
time, throughput, and dynamic allocation activity. Construction uses an lvalue input,
so its allocation count includes the required input copy. No timing threshold is
enforced because results depend on the compiler and host machine.

Configure a release build and run the dedicated target from the repository root:

```sh
cmake -S . -B build-performance \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DWAVELETPP_BUILD_CMAKE_TESTS=OFF \
    -DWAVELETPP_BUILD_PERFORMANCE_TESTS=ON
cmake --build build-performance --target waveletpp.performance
```

The benchmark defaults to 262,144 samples and 10 measured iterations. To select
different values, build the executable and invoke it directly:

```sh
cmake --build build-performance --target waveletpp.test.performance.transform
./build-performance/output/test/performance/waveletpp.test.performance.transform 1048576 5
```

Use the same compiler, build flags, machine, and power settings when comparing
results. Allocation columns report the number of ordinary C++ heap allocations and
the cumulative KiB requested per operation; they are not peak-memory measurements.

For `N` input samples and a `K`-tap wavelet, one-level DWT and IDWT are
`Theta(N * K)` time and `Theta(N)` output/workspace. A fixed-depth multilevel LPF
is also `Theta(N * K)` because the level sizes form a geometric series; its retained
workspace is `Theta(N)`. The benchmark uses filters with 2, 8, and 30 taps so both
the specialized and general kernels remain visible.
