# API reference

All public interfaces are in the `waveletpp` namespace. Include `<waveletpp.hpp>` to use the transform and filter APIs.

## Transform types

The default `waveletpp::transform` type is an alias for `waveletpp::basic_transform<double>`. The following aliases are available:

```cpp
using int_transform = basic_transform<int>;
using long_transform = basic_transform<long>;
using double_transform = basic_transform<double>;
using transform = double_transform;
```

`basic_transform<T>` supports the built-in integer and floating-point types for which `type_tool<T>` is defined.

## Constructors

```cpp
explicit basic_transform(vector_t src_data,
                         const filter_arg &filter = filter_t::db1);
explicit basic_transform(const filter_arg &filter = filter_t::db1);
```

An input signal and filter can be supplied together during construction. Alternatively, construct the object first and then set the input with `set_src_data()`.

## Member functions

| Interface | Description |
| --- | --- |
| `set_filter(filter)` | Changes the filter and returns the current object |
| `set_src_data(data)` | Changes the input data and returns the current object |
| `dwt(ext)` | Performs a one-level discrete wavelet decomposition |
| `idwt(ext)` | Reconstructs a signal from the current decomposition coefficients |
| `lpf(parameters)` | Performs multilevel threshold filtering and reconstruction |
| `filter()` | Returns the current filter enum |
| `filter_size()` | Returns the number of coefficients in the current filter |
| `data()` | Accesses the internal input, decomposition, and reconstruction data |
| `on_extend(callback)` | Installs a custom boundary-extension callback |
| `def_extend()` | Restores the default boundary-extension callback |

## Parameter and result types

### `param_t`

```cpp
struct param_t
{
    value_t threshold = type_tool<value_t>::zero;
    level_t level = 1;
};
```

`threshold` is the wavelet threshold. `level` is the decomposition level, with a maximum effective value of `8`.

### `decomposed_t`

```cpp
struct decomposed_t
{
    vector_t high;
    vector_t low;
};
```

`high` stores the high-frequency detail coefficients, and `low` stores the low-frequency approximation coefficients.

### `data_t`

| Member | Description |
| --- | --- |
| `src` | Current input data |
| `dec.low` / `dec.high` | Low- and high-frequency coefficients from the latest decomposition |
| `rec` | Result of the latest reconstruction or filtering operation |

A transform object is stateful, and its operations update these internal buffers. The values returned by `dwt()`, `idwt()`, and `lpf()` are also references to internal data. A subsequent transform may change that data, and destroying the object invalidates the references. Copy a result into an application-owned container if it must be retained.

Call `set_src_data()` before processing a new signal:

```cpp
wavelet.set_src_data(new_signal);
```

## Filter types and names

`filter_arg` can hold any of the following types:

```cpp
using filter_arg = std::variant<
    filter_t,
    std::string_view,
    std::wstring_view
>;
```

Use the following functions to convert between names and enum values:

```cpp
auto value = waveletpp::from_filter_name("db4");
auto name = waveletpp::filter_name(value);
auto wide_name = waveletpp::wfilter_name(value);
bool valid = waveletpp::check_filter("coif2");
```

- `from_filter_name()` converts a narrow or wide string name to an enum value and throws `std::invalid_argument` for an invalid name.
- `filter_name()` and `wfilter_name()` return the corresponding narrow or wide string name.
- `check_filter()` catches an invalid name and returns `false` by default. Passing `true` as its second argument preserves the exception behavior.
- `filter_arg_enum()`, `filter_arg_name()`, and `filter_arg_wname()` operate on a `filter_arg`.

The template name lookup can be used at compile time:

```cpp
constexpr auto name = waveletpp::filter_name<waveletpp::filter::db4>();
static_assert(waveletpp::is_filter_valid_v<waveletpp::filter::db4>);
```

## Filter coefficients

The following functions return a copy of the requested coefficient set:

| Interface | Coefficients |
| --- | --- |
| `lo_d(filter)` | Low-pass decomposition coefficients |
| `hi_d(filter)` | High-pass decomposition coefficients |
| `lo_r(filter)` | Low-pass reconstruction coefficients |
| `hi_r(filter)` | High-pass reconstruction coefficients |

For example:

```cpp
const auto analysis_low = waveletpp::lo_d(waveletpp::filter::sym5);
```

## Custom boundary extension

The boundary-extension callback type is:

```cpp
using ext_method_t = std::function<std::size_t(vector_t &, std::size_t)>;
```

The callback extends the data in place. Its second argument is the current filter length, and its return value is the number of samples added to each side. The transform uses that value to trim the result afterward.

```cpp
wavelet.on_extend([](auto &data, std::size_t filter_size) -> std::size_t {
    // Add samples to both sides of data here.
    // Use an even extension width before DWT so the result can be trimmed exactly.
    return 0;
});
```

The default strategy is symmetric extension. Call `def_extend()` to restore it:

```cpp
wavelet.def_extend();
```

See [Getting started and transform examples](getting-started.md) for the complete filter list and transform examples.
