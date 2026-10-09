// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include "test.h"

#include <waveletpp.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <sstream>
#include <type_traits>
#include <vector>

namespace
{

template <typename T>
std::vector<T> make_signal(std::size_t size)
{
	std::vector<T> signal;
	signal.reserve(size);
	for(std::size_t index = 0; index < size; ++index)
	{
		const auto base = static_cast<int>((index * 29 + 7) % 67) - 33;
		signal.emplace_back(static_cast<T>(base) / static_cast<T>(8));
	}
	return signal;
}

template <typename T>
long double maximum_error(const std::vector<T> &actual, const std::vector<T> &expected)
{
	if(actual.size() != expected.size())
		return std::numeric_limits<long double>::infinity();
	long double result = 0.0L;
	for(std::size_t index = 0; index < actual.size(); ++index)
	{
		const auto error = std::abs(
			static_cast<long double>(actual[index]) -
			static_cast<long double>(expected[index]));
		result = std::max(result, error);
	}
	return result;
}

template <typename T>
void floating_round_trip(long double tolerance)
{
	static_assert(std::is_floating_point_v<T>);
	const auto signal = make_signal<T>(128);
	waveletpp::basic_transform<T> transform(signal, waveletpp::filter::db4);
	transform.dwt(false);
	const auto reconstruction = transform.idwt(false);
	const auto error = maximum_error(reconstruction, signal);
	if(error <= tolerance)
		return;

	std::ostringstream message;
	message.precision(20);
	message << "floating-point round trip error " << error
	        << " exceeds tolerance " << tolerance;
	throw std::runtime_error(message.str());
}

void supported_floating_types_round_trip()
{
	floating_round_trip<float>(2e-5L);
	floating_round_trip<double>(1e-10L);
	floating_round_trip<long double>(1e-10L);
}

void long_double_path_does_not_narrow_to_double()
{
	const auto delta = std::ldexp(1.0L, -60);
	const std::vector<long double> perturbed {
		1.0L + delta, 1.0L - delta, 2.0L + delta, 2.0L - delta,
		3.0L + delta, 3.0L - delta, 4.0L + delta, 4.0L - delta
	};
	const std::vector<long double> baseline {
		1.0L, 1.0L, 2.0L, 2.0L, 3.0L, 3.0L, 4.0L, 4.0L
	};
	waveletpp::basic_transform<long double> perturbed_transform(
		perturbed, waveletpp::filter::db2);
	waveletpp::basic_transform<long double> baseline_transform(
		baseline, waveletpp::filter::db2);
	const auto perturbed_coefficients = perturbed_transform.dwt(false);
	const auto baseline_coefficients = baseline_transform.dwt(false);

	bool retained_sub_double_information = false;
	for(std::size_t index = 0; index < perturbed_coefficients.low.size(); ++index)
	{
		retained_sub_double_information = retained_sub_double_information ||
			perturbed_coefficients.low[index] != baseline_coefficients.low[index] ||
			perturbed_coefficients.high[index] != baseline_coefficients.high[index];
	}
	WAVELETPP_TEST_CHECK(retained_sub_double_information);
}

void integral_storage_is_explicitly_rejected()
{
	static_assert(not waveletpp::is_transform_value_supported_v<int>);
	static_assert(not waveletpp::is_transform_value_supported_v<unsigned int>);
	WAVELETPP_TEST_CHECK(not waveletpp::is_transform_value_supported_v<int>);
}

void thresholding_returns_finite_values_for_finite_inputs()
{
	const auto float_signal = make_signal<float>(257);
	waveletpp::basic_transform<float> float_transform(
		float_signal, waveletpp::filter::coif5);
	const auto float_filtered = float_transform.lpf({0.25F, 8});
	WAVELETPP_TEST_CHECK(std::all_of(
		float_filtered.begin(), float_filtered.end(),
		[](float value) { return std::isfinite(value); }));

	const auto long_double_signal = make_signal<long double>(257);
	waveletpp::basic_transform<long double> long_double_transform(
		long_double_signal, waveletpp::filter::coif5);
	const auto long_double_filtered = long_double_transform.lpf({0.25L, 8});
	WAVELETPP_TEST_CHECK(std::all_of(
		long_double_filtered.begin(), long_double_filtered.end(),
		[](long double value) { return std::isfinite(value); }));
}

} // namespace

int main()
{
	return waveletpp::test::run({
		{"supported floating types round trip", supported_floating_types_round_trip},
		{"long double computations do not narrow to double", long_double_path_does_not_narrow_to_double},
		{"integral transform storage is explicitly rejected", integral_storage_is_explicitly_rejected},
		{"thresholding finite inputs remains finite", thresholding_returns_finite_values_for_finite_inputs},
	});
}
