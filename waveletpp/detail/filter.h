// SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#ifndef WAVELETPP_DETAIL_FILTER_H
#define WAVELETPP_DETAIL_FILTER_H

#include <waveletpp/detail/filter_coefficients.h>

namespace waveletpp
{

template <typename CharT, filter Filter>
constexpr const CharT *filter_name()
{
	return detail::filter_name<CharT,Filter>::get();
}

template <filter Filter>
constexpr const char *filter_name()
{
	return filter_name<char,Filter>();
}

template <filter Filter>
constexpr const wchar_t *wfilter_name()
{
	return filter_name<wchar_t,Filter>();
}

template <typename CharT>
const CharT *filter_name(filter_t filter)
{
	switch( filter )
	{
#define X_MACRO(e,n) case filter::e: \
		if constexpr( std::is_same_v<CharT,char> ) \
			return n; \
		else \
			return L##n;
		WAVELETPP_FILTER_LIST
		default: break;
#undef X_MACRO
	}
	throw std::invalid_argument (
		"waveletpp::filter_name: Invalid filter"
	);
}

inline const char *filter_name(filter_t filter)
{
	return filter_name<char>(filter);
}

inline const wchar_t *wfilter_name(filter_t filter)
{
	return filter_name<wchar_t>(filter);
}

inline filter_t from_filter_name(std::string_view name)
{
#define X_MACRO(e,n) \
	if( not name.empty() and name.front() == n[0] and name == n ) \
		return filter::e;
	WAVELETPP_FILTER_LIST
#undef X_MACRO
	throw std::invalid_argument (
		"waveletpp::from_filter_name: Invalid filter name"
	);
}

inline filter_t from_filter_name(std::wstring_view name)
{
#define X_MACRO(e,n) \
	if( not name.empty() and name.front() == L##n[0] and name == L##n ) \
		return filter::e;
	WAVELETPP_FILTER_LIST
#undef X_MACRO
	throw std::invalid_argument (
		"waveletpp::from_filter_name: Invalid filter name"
	);
}

inline filter_t filter_arg_enum(const filter_arg &arg)
{
	if( arg.index() == 0 )
	{
		const auto value = std::get<0>(arg);
		if( static_cast<std::size_t>(value) < detail::filter_count )
			return value;

		throw std::invalid_argument (
			"waveletpp::filter_arg_enum: Invalid filter value"
		);
	}
	if( arg.index() == 1 )
		return from_filter_name(std::get<1>(arg));

	if( arg.index() == 2 )
		return from_filter_name(std::get<2>(arg));

	throw std::invalid_argument (
		"waveletpp::filter_arg_enum: Invalid filter argument"
	);
	// return xxx;
}

inline std::string filter_arg_name(const filter_arg &arg)
{
	return filter_name(filter_arg_enum(arg));
}

inline std::wstring filter_arg_wname(const filter_arg &arg)
{
	return wfilter_name(filter_arg_enum(arg));
}

template <typename CharT>
std::basic_string<CharT> filter_arg_name(const filter_arg &arg)
{
	if constexpr( std::is_same_v<CharT,char> )
		return filter_arg_name(arg);
	else
		return filter_arg_wname(arg);
}

inline bool check_filter(filter_arg filter, bool _throw)
{
	if( _throw )
	{
		static_cast<void>(filter_arg_enum(filter));
		return true;
	}
	try {
		static_cast<void>(filter_arg_enum(filter));
		return true;
	}
	catch(...) {}
	return false;
}

inline detail::coefficient_view lo_d_ref(filter_t filter)
{
	return detail::precise_coefficient_ref(filter, detail::coefficient_kind::lo_d);
}

inline detail::coefficient_view lo_d_ref(const filter_arg &filter)
{
	return lo_d_ref(filter_arg_enum(filter));
}

inline std::vector<double> lo_d(const filter_arg &filter)
{
	const auto coefficients = lo_d_ref(filter);
	return {coefficients.begin(), coefficients.end()};
}

inline detail::coefficient_view hi_d_ref(filter_t filter)
{
	return detail::precise_coefficient_ref(filter, detail::coefficient_kind::hi_d);
}

inline detail::coefficient_view hi_d_ref(const filter_arg &filter)
{
	return hi_d_ref(filter_arg_enum(filter));
}

inline std::vector<double> hi_d(const filter_arg &filter)
{
	const auto coefficients = hi_d_ref(filter);
	return {coefficients.begin(), coefficients.end()};
}

inline detail::coefficient_view lo_r_ref(filter_t filter)
{
	return detail::precise_coefficient_ref(filter, detail::coefficient_kind::lo_r);
}

inline detail::coefficient_view lo_r_ref(const filter_arg &filter)
{
	return lo_r_ref(filter_arg_enum(filter));
}

inline std::vector<double> lo_r(const filter_arg &filter)
{
	const auto coefficients = lo_r_ref(filter);
	return {coefficients.begin(), coefficients.end()};
}

inline detail::coefficient_view hi_r_ref(filter_t filter)
{
	return detail::precise_coefficient_ref(filter, detail::coefficient_kind::hi_r);
}

inline detail::coefficient_view hi_r_ref(const filter_arg &filter)
{
	return hi_r_ref(filter_arg_enum(filter));
}

inline std::vector<double> hi_r(const filter_arg &filter)
{
	const auto coefficients = hi_r_ref(filter);
	return {coefficients.begin(), coefficients.end()};
}

[[nodiscard]] inline size_t lo_d_size(const filter_arg &filter)
{
	return lo_d_ref(filter).size();
}

[[nodiscard]] inline size_t hi_d_size(const filter_arg &filter)
{
	return hi_d_ref(filter).size();
}

[[nodiscard]] inline size_t lo_r_size(const filter_arg &filter)
{
	return lo_r_ref(filter).size();
}

[[nodiscard]] inline size_t hi_r_size(const filter_arg &filter)
{
	return hi_r_ref(filter).size();
}

} //namespace waveletpp


#endif //WAVELETPP_DETAIL_FILTER_H
