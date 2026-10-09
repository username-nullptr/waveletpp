// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include "test.h"

#include <waveletpp.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

using signal_t = std::vector<double>;

struct filter_case
{
	waveletpp::filter value;
	std::string_view name;
};

constexpr filter_case filters[] {
#define X_MACRO(value, name) {waveletpp::filter::value, name},
	WAVELETPP_FILTER_LIST
#undef X_MACRO
};

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

signal_t make_stress_signal(std::size_t size)
{
	signal_t signal;
	signal.reserve(size);
	for(std::size_t index = 0; index < size; ++index)
	{
		const auto integer = static_cast<int>((index * 37 + 11) % 101) - 50;
		const auto alternating = index % 2 == 0 ? 0.375 : -0.625;
		signal.emplace_back(static_cast<double>(integer) / 10.0 + alternating);
	}
	return signal;
}

double maximum_error(const signal_t &actual, const signal_t &expected)
{
	if(actual.size() != expected.size())
		return std::numeric_limits<double>::infinity();
	double result = 0.0;
	for(std::size_t index = 0; index < actual.size(); ++index)
		result = std::max(result, std::abs(actual[index] - expected[index]));
	return result;
}

double energy(const signal_t &signal)
{
	double result = 0.0;
	for(const auto value : signal)
		result += value * value;
	return result;
}

[[noreturn]] void fail_measurement(
	std::string_view property,
	std::string_view filter,
	std::size_t size,
	double error,
	double tolerance,
	std::size_t level = 0
)
{
	std::ostringstream message;
	message.precision(17);
	message << property << " failed for " << filter << ", size " << size;
	if(level != 0)
		message << ", level " << level;
	message << ": error " << error << " exceeds tolerance " << tolerance;
	throw std::runtime_error(message.str());
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
	const auto signal = make_signal();

	for(const auto &entry : filters)
	{
		waveletpp::transform transform(signal, entry.value);
		const auto decomposition = transform.dwt(false);
		WAVELETPP_TEST_CHECK_EQ(decomposition.low.size(), signal.size() / 2);
		WAVELETPP_TEST_CHECK_EQ(decomposition.high.size(), signal.size() / 2);
		WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);

		const auto reconstruction = transform.idwt(false);
		const auto error = maximum_error(reconstruction, signal);
		if(error > 1e-10)
			fail_measurement("periodic DWT/IDWT round trip", entry.name,
				signal.size(), error, 1e-10);
	}
}

void periodic_impulse_basis_is_perfect_reconstruction()
{
	constexpr double tolerance = 1e-10;
	for(const auto &entry : filters)
	{
		const auto taps = waveletpp::lo_d(entry.value).size();
		const auto size = std::max<std::size_t>(2 * taps, 64);
		for(std::size_t impulse = 0; impulse < size; ++impulse)
		{
			signal_t signal(size, 0.0);
			signal[impulse] = 1.0;
			waveletpp::transform transform(signal, entry.value);
			transform.dwt(false);
			const auto reconstruction = transform.idwt(false);
			const auto error = maximum_error(reconstruction, signal);
			if(error > tolerance)
				fail_measurement("periodic impulse-basis reconstruction", entry.name,
					size, error, tolerance);
		}
	}
}

void transition_lengths_have_consistent_contracts()
{
	constexpr double tolerance = 1e-9;
	for(const auto &entry : filters)
	{
		const auto taps = waveletpp::lo_d(entry.value).size();
		for(std::size_t size = 0; size <= 2 * taps + 3; ++size)
		{
			const auto signal = make_stress_signal(size);
			waveletpp::transform transform(signal, entry.value);
			const auto decomposition = transform.dwt(false);
			if(size < 2 * taps)
			{
				WAVELETPP_TEST_CHECK_EQ(decomposition.low, signal);
				WAVELETPP_TEST_CHECK(decomposition.high.empty());
			}
			else
			{
				WAVELETPP_TEST_CHECK_EQ(decomposition.low.size(), (size + 1) / 2);
				WAVELETPP_TEST_CHECK_EQ(decomposition.high.size(), (size + 1) / 2);
			}

			const auto reconstruction = transform.idwt(false);
			WAVELETPP_TEST_CHECK_EQ(reconstruction.size(), signal.size());
			const auto error = maximum_error(reconstruction, signal);
			if(error > tolerance)
				fail_measurement("transition-length DWT/IDWT round trip", entry.name,
					size, error, tolerance);

			if(size < 2 * taps)
			{
				waveletpp::transform filter(signal, entry.value);
				WAVELETPP_TEST_CHECK_EQ(filter.lpf({0.25, 8}), signal);
			}
		}
	}
}

void default_boundary_round_trip_is_invertible()
{
	constexpr double tolerance = 1e-9;
	for(const auto &entry : filters)
	{
		const auto taps = waveletpp::lo_d(entry.value).size();
		for(const auto size : {
			std::max<std::size_t>(2 * taps, 128),
			std::max<std::size_t>(2 * taps + 2, 130)
		})
		{
			const auto signal = make_stress_signal(size);
			waveletpp::transform transform(signal, entry.value);
			transform.dwt();
			const auto reconstruction = transform.idwt();
			const auto error = maximum_error(reconstruction, signal);
			if(error > tolerance)
				fail_measurement("default-boundary DWT/IDWT round trip", entry.name,
					size, error, tolerance);
		}
	}
}

void default_boundary_db4_reference_case()
{
	constexpr double tolerance = 1e-9;
	const auto signal = make_stress_signal(128);
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	transform.dwt();
	const auto reconstruction = transform.idwt();
	const auto error = maximum_error(reconstruction, signal);
	if(error > tolerance)
		fail_measurement("default-boundary DWT/IDWT round trip", "db4",
			signal.size(), error, tolerance);
}

void odd_length_round_trip_preserves_length_and_values()
{
	constexpr double tolerance = 1e-9;
	for(const auto ext : {false, true})
	{
		const auto signal = make_stress_signal(127);
		waveletpp::transform transform(signal, waveletpp::filter::db4);
		transform.dwt(ext);
		const auto reconstruction = transform.idwt(ext);
		WAVELETPP_TEST_CHECK_EQ(reconstruction.size(), signal.size());
		const auto error = maximum_error(reconstruction, signal);
		if(error > tolerance)
			fail_measurement("odd-length DWT/IDWT round trip", "db4",
				signal.size(), error, tolerance);
	}
}

void zero_threshold_filter_is_identity_at_every_level()
{
	constexpr double tolerance = 1e-9;
	for(const auto &entry : filters)
	{
		const auto taps = waveletpp::lo_d(entry.value).size();
		for(const auto size : {
			std::max<std::size_t>(2 * taps, 128),
			std::max<std::size_t>(2 * taps + 1, 129)
		})
		{
			const auto signal = make_stress_signal(size);
			for(std::size_t level = 0; level <= 8; ++level)
			{
				waveletpp::transform transform(signal, entry.value);
				const auto filtered = transform.lpf({0.0, level});
				WAVELETPP_TEST_CHECK_EQ(filtered.size(), signal.size());
				const auto error = maximum_error(filtered, signal);
				if(error > tolerance)
					fail_measurement("zero-threshold LPF identity", entry.name,
						size, error, tolerance, level);
			}
		}
	}
}

void zero_threshold_db4_deep_reference_case()
{
	constexpr double tolerance = 1e-9;
	const auto signal = make_stress_signal(128);
	for(std::size_t level = 1; level <= 8; ++level)
	{
		waveletpp::transform transform(signal, waveletpp::filter::db4);
		const auto filtered = transform.lpf({0.0, level});
		const auto error = maximum_error(filtered, signal);
		if(error > tolerance)
			fail_measurement("zero-threshold LPF identity", "db4",
				signal.size(), error, tolerance, level);
	}
}

void haar_transform_matches_golden_values()
{
	const auto scale = std::sqrt(0.5);
	const signal_t signal {1.0, 2.0, 3.0, 4.0};
	waveletpp::transform transform(signal, waveletpp::filter::db1);
	const auto decomposition = transform.dwt(false);
	check_close(decomposition.low, {3.0 * scale, 7.0 * scale}, 2e-15);
	check_close(decomposition.high, {-scale, -scale}, 2e-15);
	check_close(transform.idwt(false), signal, 2e-15);
}

void haar_threshold_filter_matches_golden_values()
{
	const signal_t signal {1.0, 3.0, 10.0, 14.0};
	const signal_t expected {2.0, 2.0, 12.0, 12.0};
	waveletpp::transform transform(signal, waveletpp::filter::db1);
	check_close(transform.lpf({100.0, 1}), expected, 4e-15);
}

void orthogonal_filters_preserve_energy()
{
	constexpr double relative_tolerance = 1e-11;
	for(const auto &entry : filters)
	{
		if(entry.name.substr(0, 4) == "bior")
			continue;
		const auto signal = make_stress_signal(256);
		waveletpp::transform transform(signal, entry.value);
		const auto decomposition = transform.dwt(false);
		const auto input_energy = energy(signal);
		const auto relative_error = std::abs(
			energy(signal) - energy(decomposition.low) - energy(decomposition.high));
		const auto normalized_error = relative_error / input_energy;
		if(normalized_error > relative_tolerance)
			fail_measurement("orthogonal-filter energy conservation", entry.name,
				signal.size(), normalized_error, relative_tolerance);
	}
}

void coefficient_shape_mismatch_is_rejected()
{
	for(const auto mutation : {0, 1, 2})
	{
		waveletpp::transform transform(waveletpp::filter::db4);
		if(mutation != 0)
		{
			transform.set_src_data(make_stress_signal(128));
			transform.dwt(false);
			if(mutation == 1)
				transform.data().dec.high.pop_back();
			else
			{
				transform.data().dec.low.clear();
				transform.data().dec.high.clear();
			}
		}
		else
		{
			transform.data().dec.low = {1.0, 2.0};
			transform.data().dec.high = {0.5};
		}

		bool threw = false;
		try
		{
			static_cast<void>(transform.idwt(false));
		}
		catch(const std::invalid_argument &)
		{
			threw = true;
		}
		WAVELETPP_TEST_CHECK(threw);
	}
}

void invalid_custom_extension_width_is_rejected()
{
	const auto signal = make_stress_signal(128);
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	transform.on_extend([](auto &data, std::size_t) {
		data.insert(data.begin(), data.front());
		data.emplace_back(data.back());
		return std::size_t {1};
	});

	bool threw = false;
	try
	{
		static_cast<void>(transform.dwt());
	}
	catch(const std::invalid_argument &)
	{
		threw = true;
	}
	WAVELETPP_TEST_CHECK(threw);
	WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);

	transform.on_extend([](auto &data, std::size_t) {
		data.insert(data.begin(), data.front());
		data.emplace_back(data.back());
		return std::size_t {2};
	});
	threw = false;
	try
	{
		static_cast<void>(transform.dwt());
	}
	catch(const std::invalid_argument &)
	{
		threw = true;
	}
	WAVELETPP_TEST_CHECK(threw);
	WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);

	threw = false;
	try
	{
		transform.on_extend(waveletpp::transform::ext_method_t {});
	}
	catch(const std::invalid_argument &)
	{
		threw = true;
	}
	WAVELETPP_TEST_CHECK(threw);
}

void extension_exception_preserves_transform_state()
{
	const auto signal = make_stress_signal(128);
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	transform.on_extend([](auto &data, std::size_t) -> std::size_t {
		data.emplace_back(12345.0);
		throw std::runtime_error("extension failed");
	});

	bool threw = false;
	try
	{
		static_cast<void>(transform.dwt());
	}
	catch(const std::runtime_error &)
	{
		threw = true;
	}
	WAVELETPP_TEST_CHECK(threw);
	WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);
}

void invalid_thresholds_are_rejected_without_mutation()
{
	const auto signal = make_stress_signal(128);
	for(const auto threshold : {
		-1.0,
		std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::quiet_NaN()
	})
	{
		waveletpp::transform transform(signal, waveletpp::filter::db4);
		bool threw = false;
		try
		{
			static_cast<void>(transform.lpf({threshold, 2}));
		}
		catch(const std::invalid_argument &)
		{
			threw = true;
		}
		WAVELETPP_TEST_CHECK(threw);
		WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);
		WAVELETPP_TEST_CHECK(transform.data().dec.low.empty());
		WAVELETPP_TEST_CHECK(transform.data().dec.high.empty());
		WAVELETPP_TEST_CHECK(transform.data().rec.empty());
	}
}

void decomposition_level_limits_are_deterministic()
{
	const auto signal = make_stress_signal(256);
	waveletpp::transform level_eight(signal, waveletpp::filter::sym5);
	const auto expected = level_eight.lpf({0.25, 8});

	waveletpp::transform oversized_level(signal, waveletpp::filter::sym5);
	WAVELETPP_TEST_CHECK_EQ(oversized_level.lpf({0.25, 999}), expected);

	waveletpp::transform level_zero(signal, waveletpp::filter::sym5);
	WAVELETPP_TEST_CHECK_EQ(level_zero.lpf({0.25, 0}), signal);
	WAVELETPP_TEST_CHECK_EQ(level_zero.data().rec, signal);
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

void odd_length_preserves_source()
{
	const auto signal = make_signal(127);
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	const auto decomposition = transform.dwt(false);

	WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);
	WAVELETPP_TEST_CHECK_EQ(decomposition.low.size(), (signal.size() + 1) / 2);
	WAVELETPP_TEST_CHECK_EQ(decomposition.high.size(), (signal.size() + 1) / 2);

	const auto reconstruction = transform.idwt(false);
	WAVELETPP_TEST_CHECK_EQ(reconstruction.size(), signal.size());
	check_close(reconstruction, signal, 1e-9);
}

void short_signal_is_unchanged()
{
	const auto signal = make_signal(8);
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	const auto decomposition = transform.dwt();

	WAVELETPP_TEST_CHECK_EQ(decomposition.low, signal);
	WAVELETPP_TEST_CHECK(decomposition.high.empty());
	WAVELETPP_TEST_CHECK_EQ(transform.idwt(), signal);
	WAVELETPP_TEST_CHECK_EQ(transform.lpf({0.25, 8}), signal);
	WAVELETPP_TEST_CHECK_EQ(transform.data().rec, signal);
}

void custom_extension_runs_once_per_level()
{
	const auto signal = make_signal();
	waveletpp::transform default_transform(signal, waveletpp::filter::db4);
	const auto expected = default_transform.lpf({0.1, 8});

	std::size_t calls = 0;
	waveletpp::transform transform(signal, waveletpp::filter::db4);
	transform.on_extend([&calls](auto &data, std::size_t filter_size) {
		++calls;
		std::vector<double> extended(data.size() + filter_size * 2);
		std::copy(data.begin(), data.end(), extended.begin() + filter_size);
		std::reverse_copy(
			data.begin(), data.begin() + filter_size, extended.begin()
		);
		std::reverse_copy(
			data.end() - filter_size, data.end(),
			extended.begin() + filter_size + data.size()
		);
		data = std::move(extended);
		return filter_size;
	});

	const auto filtered = transform.lpf({0.1, 8});
	WAVELETPP_TEST_CHECK_EQ(calls, std::size_t {8});
	WAVELETPP_TEST_CHECK_EQ(filtered.size(), signal.size());
	check_close(filtered, expected, 1e-12);
}

void deep_filtering_preserves_length()
{
	for(const auto size : {std::size_t {127}, std::size_t {128}, std::size_t {1024}})
	{
		const auto signal = make_signal(size);
		waveletpp::transform transform(signal, waveletpp::filter::coif5);
		const auto filtered = transform.lpf({0.1, 8});

		WAVELETPP_TEST_CHECK_EQ(filtered.size(), signal.size());
		WAVELETPP_TEST_CHECK_EQ(transform.data().src, signal);
		WAVELETPP_TEST_CHECK(std::all_of(filtered.begin(), filtered.end(),
			[](double value) { return std::isfinite(value); }));
	}
}

} // namespace

int main()
{
	return waveletpp::test::run({
		{"DWT and IDWT round trip", transform_round_trip},
		{"periodic impulse basis is perfectly reconstructed", periodic_impulse_basis_is_perfect_reconstruction},
		{"transition lengths have consistent contracts", transition_lengths_have_consistent_contracts},
		{"db4 default-boundary reference case is invertible", default_boundary_db4_reference_case},
		{"default boundary round trip is invertible", default_boundary_round_trip_is_invertible},
		{"odd-length round trip preserves length and values", odd_length_round_trip_preserves_length_and_values},
		{"db4 deep zero-threshold reference case is identity", zero_threshold_db4_deep_reference_case},
		{"zero-threshold filtering is identity at every level", zero_threshold_filter_is_identity_at_every_level},
		{"Haar transform matches golden values", haar_transform_matches_golden_values},
		{"Haar threshold filter matches golden values", haar_threshold_filter_matches_golden_values},
		{"orthogonal filters preserve energy", orthogonal_filters_preserve_energy},
		{"coefficient shape mismatch is rejected", coefficient_shape_mismatch_is_rejected},
		{"invalid custom extension width is rejected", invalid_custom_extension_width_is_rejected},
		{"extension exceptions preserve transform state", extension_exception_preserves_transform_state},
		{"invalid thresholds are rejected without mutation", invalid_thresholds_are_rejected_without_mutation},
		{"decomposition level limits are deterministic", decomposition_level_limits_are_deterministic},
		{"low-pass filter preserves source", low_pass_filter_preserves_source},
		{"boundary extension preserves inputs", boundary_extension_preserves_inputs},
		{"transform value semantics", value_semantics},
		{"setters and empty input", setters_and_empty_input},
		{"odd-length transform preserves source", odd_length_preserves_source},
		{"short signal is unchanged", short_signal_is_unchanged},
		{"custom extension runs once per level", custom_extension_runs_once_per_level},
		{"deep filtering preserves length", deep_filtering_preserves_length},
	});
}
