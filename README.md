# Waveletpp

Waveletpp is a header-only C++17 discrete wavelet transform library for one-dimensional signals. It provides the discrete wavelet transform (DWT), inverse discrete wavelet transform (IDWT), and wavelet-threshold low-pass filtering.

## Features

- Header-only; no library binary is required
- DWT, IDWT, and wavelet-threshold filtering with up to eight levels
- 44 built-in Daubechies, Biorthogonal, Coiflets, and Symlets filters
- Customizable signal boundary extension
- An installable CMake package and the `waveletpp::waveletpp` exported target

## Quick start

```cpp
#include <waveletpp.hpp>

#include <iostream>
#include <vector>

int main()
{
    std::vector<double> signal {
        1.0, 1.2, 0.8, 1.1, 4.0, 4.2, 3.8, 4.1,
        1.0, 1.1, 0.9, 1.2, 4.1, 3.9, 4.2, 4.0
    };

    waveletpp::transform wavelet(signal, waveletpp::filter::db2);

    const auto &filtered = wavelet.lpf({0.25, 2});
    for (double value : filtered)
        std::cout << value << '\n';
}
```

When invoking the compiler directly, add the repository root to the header search path:

```sh
c++ -std=c++17 -I/path/to/waveletpp main.cpp -o example
```

When adding Waveletpp as a CMake subdirectory, link its interface target:

```cmake
set(WAVELETPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/waveletpp)

target_link_libraries(example PRIVATE waveletpp::waveletpp)
```

## Documentation

- [Runnable examples: basic, CSV, and advanced](examples/README.md)
- [Documentation index](doc/README.md)
- [Getting started and transform examples](doc/getting-started.md)
- [API reference](doc/api.md)
- [CMake integration](doc/cmake.md)

## Requirements

- C++17 or later
- CMake 3.15 or later when using CMake
- GCC 7, Clang 5, AppleClang 9, Visual Studio 2017, or a newer compiler

## License

This project is distributed under the [MIT License](LICENSE.txt).
