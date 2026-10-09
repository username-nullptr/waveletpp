// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include "test.h"

#include <waveletpp.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{

using signal_t = std::vector<double>;

signal_t make_signal(std::size_t size = 128)
{
	constexpr double pi = 3.14159265358979323846;
	signal_t signal;
	signal.reserve(size);
	for(std::size_t index = 0; index < size; ++index)
	{
		const auto phase = 2.0 * pi * static_cast<double>(index) /
			static_cast<double>(size);
		signal.emplace_back(
			std::sin(phase) + 0.25 * std::cos(5.0 * phase) +
			0.05 * static_cast<double>(index % 7));
	}
	return signal;
}

void check_close(const signal_t &actual, const signal_t &expected, double tolerance)
{
	WAVELETPP_TEST_CHECK_EQ(actual.size(), expected.size());
	for(std::size_t index = 0; index < actual.size(); ++index)
	{
		const auto error = std::abs(actual[index] - expected[index]);
		if(error > tolerance)
		{
			std::ostringstream message;
			message << "sample " << index << " differs by " << error
				<< " (tolerance " << tolerance << ')';
			throw std::runtime_error(message.str());
		}
	}
}

void transform_round_trip()
{
	constexpr std::array filters {
		waveletpp::filter::db1,
		waveletpp::filter::db4,
		waveletpp::filter::bior44,
		waveletpp::filter::coif3,
		waveletpp::filter::sym5,
	};
	const auto signal = make_signal();

	for(const auto filter : filters)
	{
		waveletpp::transform transform(signal, filter);
		const auto decomposition = transform.dwt(false);
		WAVELETPP_TEST_CHECK_EQ(decomposition.low.size(), signal.size() / 2);
		WAVELETPP_TEST_CHECK_EQ(decomposition.high.size(), signal.size() / 2);
		WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);

		const auto reconstruction = transform.idwt(false);
		check_close(reconstruction, signal, 1e-3);
	}
}

void low_pass_filter_preserves_source()
{
	const auto signal = make_signal();
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	const auto filtered = transform.lpf({0.2, 3});

	WAVELETPP_TEST_CHECK_EQ(filtered.size(), signal.size());
	WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);
	WAVELETPP_TEST_CHECK(std::all_of(filtered.begin(), filtered.end(),
		[](double value) { return std::isfinite(value); }));

	waveletpp::transform identity(signal, waveletpp::filter::db4);
	check_close(identity.lpf({0.0, 2}), signal, 1e-10);
}

void boundary_extension_preserves_inputs()
{
	const auto signal = make_signal();
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	const auto decomposition = transform.dwt();

	WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);
	const auto low = decomposition.low;
	const auto high = decomposition.high;
	static_cast<void>(transform.idwt());
	WAVELETPP_TEST_CHECK_EQ(transform.data().dec.low, low);
	WAVELETPP_TEST_CHECK_EQ(transform.data().dec.high, high);
}

void value_semantics()
{
	const auto signal = make_signal(64);
	waveletpp::transform original(signal, waveletpp::filter::sym5);

	waveletpp::transform copied(original);
	WAVELETPP_TEST_CHECK_EQ(copied.filter(), original.filter());
	WAVELETPP_TEST_CHECK_EQ(copied.data().src, signal);

	waveletpp::transform copy_assigned(waveletpp::filter::db1);
	copy_assigned = original;
	WAVELETPP_TEST_CHECK_EQ(copy_assigned.filter(), original.filter());
	WAVELETPP_TEST_CHECK_EQ(copy_assigned.data().src, signal);

	waveletpp::transform moved(std::move(copied));
	WAVELETPP_TEST_CHECK_EQ(moved.filter(), original.filter());
	WAVELETPP_TEST_CHECK_EQ(moved.data().src, signal);

	waveletpp::transform move_assigned(waveletpp::filter::db2);
	move_assigned = std::move(moved);
	WAVELETPP_TEST_CHECK_EQ(move_assigned.filter(), original.filter());
	WAVELETPP_TEST_CHECK_EQ(move_assigned.data().src, signal);
}

void setters_and_empty_input()
{
	waveletpp::transform transform;
	WAVELETPP_TEST_CHECK_EQ(transform.filter(), waveletpp::filter::db1);
	WAVELETPP_TEST_CHECK_EQ(transform.filter_size(), std::size_t {2});
	WAVELETPP_TEST_CHECK(transform.dwt().low.empty());
	WAVELETPP_TEST_CHECK(transform.idwt().empty());

	const auto signal = make_signal(64);
	transform.set_filter("coif2").set_src_data(signal);
	WAVELETPP_TEST_CHECK_EQ(transform.filter(), waveletpp::filter::coif2);
	WAVELETPP_TEST_CHECK_EQ(transform.filter_size(), waveletpp::lo_d("coif2").size());
	WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);
}

} // namespace

int main()
{
	return waveletpp::test::run({
		{"DWT and IDWT round trip", transform_round_trip},
		{"low-pass filter preserves source", low_pass_filter_preserves_source},
		{"boundary extension preserves inputs", boundary_extension_preserves_inputs},
		{"transform value semantics", value_semantics},
		{"setters and empty input", setters_and_empty_input},
	});
}
