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