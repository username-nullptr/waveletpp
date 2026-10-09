# Examples

The examples are split into three independent programs. Start with `basic` and
move on only when you need file output or lower-level transform details.

| Program | Purpose |
| --- | --- |
| [`basic.cpp`](basic.cpp) | Minimal wavelet filtering example for first-time users |
| [`csv.cpp`](csv.cpp) | Generates 8,192 signal samples and writes chart-ready CSV data |
| [`advanced.cpp`](advanced.cpp) | Shows filter selection, DWT/IDWT coefficients, reconstruction, and quality metrics |

## Build

From the repository root:

```sh
cmake -S . -B build -DWAVELETPP_BUILD_EXAMPLES=ON
cmake --build build
```

The executables are written to `build/output/examples`:

```sh
./build/output/examples/basic
./build/output/examples/csv
./build/output/examples/advanced sym5
```

## Visualize the CSV data

`csv` writes `wavelet_signal.csv` in the current directory by default. An
optional path can be supplied on the command line:

```sh
./build/output/examples/csv results.csv
```

Open the file in Microsoft Office or WPS, select the `time_seconds`, `clean`,
`noisy`, and `filtered` columns, and insert a line chart. The data is generated
deterministically, so repeated runs are directly comparable.
