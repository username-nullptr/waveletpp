// SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#ifndef WAVELETPP_TRANSFORM_H
#define WAVELETPP_TRANSFORM_H

#include <waveletpp/filter.h>
#include <waveletpp/types.h>
#include <type_traits>
#include <functional>

namespace waveletpp
{

template <typename T>
constexpr bool is_transform_value_supported_v = std::is_floating_point_v<T>;

template <typename T>
class WAVELETPP_TAPI basic_transform
{
	static_assert(is_transform_value_supported_v<T>,
		"waveletpp::basic_transform requires a floating-point value type"
	);

public:
	using value_t = T;
	using vector_t = std::vector<value_t>;

	using level_t = size_t;
	using ext_method_t = std::function<size_t(vector_t&,size_t)>;

	struct decomposed_t
	{
		vector_t high {};
		vector_t low {};
	};
	struct data_t
	{
		vector_t src {};
		vector_t rec {};
		decomposed_t dec {};
	};
	struct param_t
	{
		value_t threshold = type_tool<value_t>::zero;
		level_t level = 1;
	};

public:
	explicit basic_transform(vector_t src_data, const filter_arg &filter = filter_t::db1);
	explicit basic_transform(const filter_arg &filter = filter_t::db1);
	~basic_transform();

	basic_transform(const basic_transform &other);
	basic_transform &operator=(const basic_transform &other);

	basic_transform(basic_transform &&other) noexcept ;
	basic_transform &operator=(basic_transform &&other) noexcept;

public:
	basic_transform &set_filter(const filter_arg &filter);
	basic_transform &set_src_data(vector_t src_data);

	decomposed_t &dwt(bool ext = true);
	vector_t &idwt(bool ext = true);
	vector_t &lpf(param_t param = {});

public:
	[[nodiscard]] filter_t filter() const noexcept;
	[[nodiscard]] size_t filter_size() const noexcept;

	[[nodiscard]] const data_t &data() const noexcept;
	[[nodiscard]] data_t &data() noexcept;

public:
	basic_transform &on_extend(ext_method_t ext);
	basic_transform &def_extend();

private:
	class impl;
	impl *m_impl;
};

using float_transform = basic_transform<float>;
using double_transform = basic_transform<double>;
using long_double_transform = basic_transform<long double>;
using transform = double_transform;

} //namespace waveletpp
#include <waveletpp/detail/transform.h>


#endif //WAVELETPP_TRANSFORM_H
