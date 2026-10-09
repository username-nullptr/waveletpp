# CMake integration

Waveletpp requires CMake 3.15 or later. Its `waveletpp::waveletpp` interface target provides the C++17 requirement and header search path to consumers.

## Use as a subdirectory

Place the project in your source tree, for example under `third_party/waveletpp`, then add the subdirectory and link the target:

```cmake
cmake_minimum_required(VERSION 3.15)
project(example LANGUAGES CXX)

set(WAVELETPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/waveletpp)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE waveletpp::waveletpp)
```

## Use the installed package

Configure and install Waveletpp first:

```sh
cmake -S . -B build -DWAVELETPP_BUILD_EXAMPLES=OFF
cmake --install build --prefix /path/to/install
```

Find and link the package from the consuming project:

```cmake
cmake_minimum_required(VERSION 3.15)
project(example LANGUAGES CXX)

find_package(Waveletpp 0.2 CONFIG REQUIRED)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE waveletpp::waveletpp)
```

If Waveletpp was installed outside a standard system prefix, provide its installation prefix when configuring the consuming project:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/install
cmake --build build
```

## Build the repository examples

Enable the example programs explicitly:

```sh
cmake -S . -B build -DWAVELETPP_BUILD_EXAMPLES=ON
cmake --build build
```

The example executables are placed under `output/examples` in the build directory.

## Build and run the tests

Enable CTest while configuring, then build and run the registered functional and
CMake package-consumer tests:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Set `WAVELETPP_ENABLE_TEST_SANITIZERS=ON` to instrument functional tests with
AddressSanitizer and, where supported, UndefinedBehaviorSanitizer.

## CMake options

| Option | Default | Description |
| --- | --- | --- |
| `BUILD_TESTING` | `OFF` | Builds and registers the functional tests |
| `WAVELETPP_BUILD_CMAKE_TESTS` | value of `BUILD_TESTING` | Tests the installed CMake package |
| `WAVELETPP_ENABLE_TEST_SANITIZERS` | `OFF` | Enables AddressSanitizer and, where available, UndefinedBehaviorSanitizer for tests |
| `WAVELETPP_BUILD_EXAMPLES` | `OFF` | Builds the example programs |
| `WAVELETPP_STRICT_WARNINGS` | `ON` | Enables strict warnings for first-party compiled targets |
| `WAVELETPP_WARNINGS_AS_ERRORS` | `OFF` | Treats first-party warnings as errors |
| `WAVELETPP_USE_LIBCXX` | `OFF` | Uses libc++ with Clang |
| `WAVELETPP_USE_LLD` | `OFF` | Uses the lld linker with Clang |
| `WAVELETPP_ENABLE_LTO` | `OFF` | Enables link-time optimization |

`WAVELETPP_USE_LIBCXX` and `WAVELETPP_USE_LLD` are supported only with Clang or AppleClang. When `WAVELETPP_ENABLE_LTO` is enabled, CMake checks whether the current toolchain supports link-time optimization.

## Compiler requirements

The project configuration checks the following minimum versions:

- GCC 7
- Clang 5
- AppleClang 9
- MSVC 19.10 (Visual Studio 2017)

Return to the [documentation index](README.md).
