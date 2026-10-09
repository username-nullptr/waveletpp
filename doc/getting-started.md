# Getting started and transform examples

## Include Waveletpp

Waveletpp is a header-only library. Include its single entry-point header in your source code:

```cpp
#include <waveletpp.hpp>
```

When invoking the compiler directly, add the repository root to the header search path:

```sh
c++ -std=c++17 -I/path/to/waveletpp main.cpp -o example
```

For CMake projects, see [CMake integration](cmake.md).

For complete runnable programs, see the three-part [examples guide](../examples/README.md):
the minimal first example, chart-ready CSV output, and advanced transform usage.

## Discrete wavelet transform

`dwt()` performs a one-level discrete wavelet decomposition and stores the result in `data().dec`. The `low` vector contains the approximation coefficients, and the `high` vector contains the detail coefficients.

```cpp
#include <waveletpp.hpp>

int main()
{
    waveletpp::transform wavelet({
        1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0
    }, waveletpp::filter::db2);

    const auto &coefficients = wavelet.dwt();
    const auto &approximation = coefficients.low;
    const auto &detail = coefficients.high;

    // Use approximation and detail here.
}
```

`idwt()` reconstructs a signal from the current `data().dec.low` and `data().dec.high` coefficients and stores the result in `data().rec`. This allows an application to modify the coefficients after DWT and then reconstruct the signal.

The `ext` argument of `dwt()` and `idwt()` defaults to `true`, which enables boundary extension. Pass `false` to disable it.

## Wavelet-threshold filtering

`lpf()` performs DWT at each level, sets high-frequency coefficients whose absolute values are below the threshold to zero, and then reconstructs the signal:

```cpp
#include <waveletpp.hpp>

#include <vector>

int main()
{
    std::vector<double> signal {
        1.0, 1.2, 0.8, 1.1, 4.0, 4.2, 3.8, 4.1,
        1.0, 1.1, 0.9, 1.2, 4.1, 3.9, 4.2, 4.0
    };

    waveletpp::transform wavelet(signal, "db2");

    waveletpp::transform::param_t parameters;
    parameters.threshold = 0.25;
    parameters.level = 2;

    const auto &filtered = wavelet.lpf(parameters);
}
```

The parameters follow these rules:

- `threshold` defaults to zero and must be finite and non-negative.
- `level` defaults to `1`. A value of `0` returns the current input without decomposition.
- The maximum decomposition level is `8`; larger values are treated as `8`.
- No decomposition is performed when the input length is less than twice the filter length.

## Select a filter

Constructors and `set_filter()` accept an enum, `std::string_view`, or `std::wstring_view`:

```cpp
waveletpp::transform first(waveletpp::filter::coif3);
waveletpp::transform second("bior3.5");
waveletpp::transform third(L"sym8");
```

The following filters are available:

| Family | Enum values | String names |
| --- | --- | --- |
| Daubechies | `db1` through `db15` | `"db1"` through `"db15"` |
| Biorthogonal | `bior11`, `bior13`, `bior15`, `bior22`, `bior24`, `bior26`, `bior28`, `bior31`, `bior33`, `bior35`, `bior37`, `bior39`, `bior44`, `bior55`, `bior68` | `"bior1.1"`, `"bior1.3"`, and so on |
| Coiflets | `coif1` through `coif5` | `"coif1"` through `"coif5"` |
| Symlets | `sym2` through `sym10` | `"sym2"` through `"sym10"` |

Filter names are case-sensitive. An invalid name passed to a constructor, `set_filter()`, or `from_filter_name()` causes `std::invalid_argument` to be thrown.

See the [API reference](api.md) for more interfaces and state semantics.
