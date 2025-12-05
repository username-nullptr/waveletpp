
/************************************************************************************
*                                                                                   *
*   Copyright (c) 2024 Xiaoqiang <username_nullptr@163.com>                         *
*                                                                                   *
*   This file is part of LIBGS                                                      *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining a copy    *
*   of this software and associated documentation files (the "Software"), to deal   *
*   in the Software without restriction, including without limitation the rights    *
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       *
*   copies of the Software, and to permit persons to whom the Software is           *
*   furnished to do so, subject to the following conditions:                        *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef WAVELETPP_FILTER_H
#define WAVELETPP_FILTER_H

#include <waveletpp/global.h>
#include <string_view>
#include <variant>
#include <vector>

namespace waveletpp
{

#define WAVELETPP_FILTER_LIST \
	X_MACRO( db1    , "db1"     ) \
	X_MACRO( db2    , "db2"     ) \
	X_MACRO( db3    , "db3"     ) \
	X_MACRO( db4    , "db4"     ) \
	X_MACRO( db5    , "db5"     ) \
	X_MACRO( db6    , "db6"     ) \
	X_MACRO( db7    , "db7"     ) \
	X_MACRO( db8    , "db8"     ) \
	X_MACRO( db9    , "db9"     ) \
	X_MACRO( db10   , "db10"    ) \
	X_MACRO( db11   , "db11"    ) \
	X_MACRO( db12   , "db12"    ) \
	X_MACRO( db13   , "db13"    ) \
	X_MACRO( db14   , "db14"    ) \
	X_MACRO( db15   , "db15"    ) \
	X_MACRO( bior11 , "bior1.1" ) \
	X_MACRO( bior13 , "bior1.3" ) \
	X_MACRO( bior15 , "bior1.5" ) \
	X_MACRO( bior22 , "bior2.2" ) \
	X_MACRO( bior24 , "bior2.4" ) \
	X_MACRO( bior26 , "bior2.6" ) \
	X_MACRO( bior28 , "bior2.8" ) \
	X_MACRO( bior31 , "bior3.1" ) \
	X_MACRO( bior33 , "bior3.3" ) \
	X_MACRO( bior35 , "bior3.5" ) \
	X_MACRO( bior37 , "bior3.7" ) \
	X_MACRO( bior39 , "bior3.9" ) \
	X_MACRO( bior44 , "bior4.4" ) \
	X_MACRO( bior55 , "bior5.5" ) \
	X_MACRO( bior68 , "bior6.8" ) \
	X_MACRO( coif1  , "coif1"   ) \
	X_MACRO( coif2  , "coif2"   ) \
	X_MACRO( coif3  , "coif3"   ) \
	X_MACRO( coif4  , "coif4"   ) \
	X_MACRO( coif5  , "coif5"   ) \
	X_MACRO( sym2   , "sym2"    ) \
	X_MACRO( sym3   , "sym3"    ) \
	X_MACRO( sym4   , "sym4"    ) \
	X_MACRO( sym5   , "sym5"    ) \
	X_MACRO( sym6   , "sym6"    ) \
	X_MACRO( sym7   , "sym7"    ) \
	X_MACRO( sym8   , "sym8"    ) \
	X_MACRO( sym9   , "sym9"    ) \
	X_MACRO( sym10  , "sym10"   )

enum class filter
{
#define X_MACRO(e,n) e,
	WAVELETPP_FILTER_LIST
#undef X_MACRO
};
using filter_t = filter;

template <typename CharT, filter Filter>
[[nodiscard]] constexpr const CharT *filter_name();

template <filter Filter>
[[nodiscard]] constexpr const char *filter_name();

template <filter Filter>
[[nodiscard]] constexpr const wchar_t *wfilter_name();

template <typename CharT>
[[nodiscard]] WAVELETPP_TAPI const CharT *filter_name(filter_t filter);

[[nodiscard]] WAVELETPP_VAPI const char *filter_name(filter_t filter);
[[nodiscard]] WAVELETPP_VAPI const wchar_t *wfilter_name(filter_t filter);

[[nodiscard]] WAVELETPP_API filter_t from_filter_name(std::string_view name);
[[nodiscard]] WAVELETPP_API filter_t from_filter_name(std::wstring_view name);

using filter_arg = std::variant <
	filter_t, std::string_view, std::wstring_view
>;
using filter_arg_t = filter_arg;

[[nodiscard]] WAVELETPP_API filter_t filter_arg_enum(const filter_arg &arg);
[[nodiscard]] WAVELETPP_API std::string_view filter_arg_name(const filter_arg &arg);
[[nodiscard]] WAVELETPP_API std::wstring_view filter_arg_wname(const filter_arg &arg);

template <typename CharT>
[[nodiscard]] WAVELETPP_TAPI std::basic_string_view<CharT> filter_arg_name(const filter_arg &arg);

template <filter_t>
struct is_filter_valid : std::false_type {};

#define X_MACRO(e,n) template <> \
	struct is_filter_valid<filter::e> : std::true_type {};
WAVELETPP_FILTER_LIST
#undef X_MACRO

template <filter_t Filter>
constexpr bool is_filter_valid_v = is_filter_valid<Filter>::value;

[[nodiscard]] WAVELETPP_API bool check_filter(filter_arg filter, bool _throw = false);

[[nodiscard]] WAVELETPP_API std::vector<double> lo_d(const filter_arg &filter);
[[nodiscard]] WAVELETPP_API std::vector<double> hi_d(const filter_arg &filter);
[[nodiscard]] WAVELETPP_API std::vector<double> lo_r(const filter_arg &filter);
[[nodiscard]] WAVELETPP_API std::vector<double> hi_r(const filter_arg &filter);

} //namespace waveletpp
#include <waveletpp/detail/filter.h>


#endif //WAVELETPP_FILTER_H