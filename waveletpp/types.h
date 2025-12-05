
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

#ifndef WAVELETPP_TYPES_H
#define WAVELETPP_TYPES_H

#include <cmath>

namespace waveletpp
{

template <typename>
struct type_tool;

#define WAVELETPP_TYPE_TOOL(T,t,z) \
	template <> struct type_tool<T> { \
		static constexpr T threshold = t; \
		static constexpr T zero = z; \
		[[nodiscard]] static T abs(const T &v) { \
			return WAVELETPP_ABS(v); \
		} \
	} \

#define WAVELETPP_ABS(v)  std::abs(v)

WAVELETPP_TYPE_TOOL(char, 1, 0);
WAVELETPP_TYPE_TOOL(short, 1, 0);
WAVELETPP_TYPE_TOOL(int, 1, 0);
WAVELETPP_TYPE_TOOL(long, 1, 0);
WAVELETPP_TYPE_TOOL(long long, 1, 0);

WAVELETPP_TYPE_TOOL(float, 0.1f, 0.0f);
WAVELETPP_TYPE_TOOL(double, 0.1, 0.0);
WAVELETPP_TYPE_TOOL(long double, 0.1l, 0.0l);

#undef WAVELETPP_ABS
#define WAVELETPP_ABS(v)  v

WAVELETPP_TYPE_TOOL(unsigned char, 1, 0);
WAVELETPP_TYPE_TOOL(unsigned short, 1, 0);
WAVELETPP_TYPE_TOOL(unsigned int, 1, 0);
WAVELETPP_TYPE_TOOL(unsigned long, 1, 0);
WAVELETPP_TYPE_TOOL(unsigned long long, 1, 0);

#undef WAVELETPP_TYPE_TOOL

} //namespace waveletpp


#endif //WAVELETPP_TYPES_H