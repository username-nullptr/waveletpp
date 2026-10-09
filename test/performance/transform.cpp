// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include <waveletpp.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace allocation_counter
{

std::size_t count = 0;
std::size_t bytes = 0;
bool enabled = false;

void record(std::size_t size) noexcept
{
	if(enabled)
	{
		++count;
		bytes += size;
	}
}

void *allocate(std::size_t size)
{
	record(size);
	if(void *memory = std::malloc(size == 0 ? 1 : size))
		return memory;
	throw std::bad_alloc();
}

void reset() noexcept
{
	count = 0;
	bytes = 0;
}

} // namespace allocation_counter

void *operator new(std::size_t size)
{
	return allocation_counter::allocate(size);
}

void *operator new[](std::size_t size)
{
	return allocation_counter::allocate(size);
}

void operator delete(void *memory) noexcept
{
	std::free(memory);
}

void operator delete[](void *memory) noexcept
{
	std::free(memory);
}

void operator delete(void *memory, std::size_t) noexcept
{
	std::free(memory);
}

void operator delete[](void *memory, std::size_t) noexcept
{
	std::free(memory);
}

namespace
{

using clock_t = std::chrono::steady_clock;
using signal_t = std::vector<double>;

constexpr std::size_t default_sample_count = 1U << 18U;
constexpr std::size_t default_iterations = 10;
constexpr std::size_t warmup_iterations = 3;
constexpr std::size_t minimum_sample_count = 64;
constexpr double pi = 3.14159265358979323846;

volatile double checksum = 0.0;

struct result_t
{
	std::string_view operation;
	std::string_view filter;
	std::string_view mode;
	std::size_t level;
	double nanoseconds;
	double allocations;
	double allocated_bytes;
};

signal_t make_signal(std::size_t size)
{
	signal_t signal;
	signal.reserve(size);

	for(std::size_t index=0; index<size; ++index)
	{
		const auto phase = 2.0 * pi * static_cast<double>(index) /
		                   static_cast<double>(size);
		signal.emplace_back(
			std::sin(phase) + 0.25 * std::cos(5.0 * phase) +
			0.05 * static_cast<double>(index % 7)
		);
	}
	return signal;
}

std::size_t parse_positive(std::string_view text, std::string_view name)
{
	if( text.empty() or text.front() == '-' )
		throw std::invalid_argument(std::string(name) + " must be a positive integer");

	std::size_t parsed_characters = 0;
	unsigned long long value = 0;
	try {
		value = std::stoull(std::string(text), &parsed_characters);
	}
	catch(const std::exception &) {
		throw std::invalid_argument(std::string(name) + " must be a positive integer");
	}

	if( parsed_characters != text.size() or value == 0 or
	    value > std::numeric_limits<std::size_t>::max() )
	{
		throw std::invalid_argument(std::string(name) + " must be a positive integer");
	}
	return static_cast<std::size_t>(value);
}

template <typename Operation>
result_t measure(
	std::string_view operation,
	std::string_view filter,
	std::string_view mode,
	std::size_t level,
	std::size_t iterations,
	Operation &&run
)
{
	for(std::size_t iteration=0; iteration<warmup_iterations; ++iteration)
		checksum = checksum + run();

	allocation_counter::reset();
	allocation_counter::enabled = true;
	const auto begin = clock_t::now();
	for(std::size_t iteration=0; iteration<iterations; ++iteration)
		checksum = checksum + run();
	const auto end = clock_t::now();
	allocation_counter::enabled = false;

	const auto elapsed = std::chrono::duration<double,std::nano>(end - begin).count();
	return {
		operation,
		filter,
		mode,
		level,
		elapsed / static_cast<double>(iterations),
		static_cast<double>(allocation_counter::count) /
			static_cast<double>(iterations),
		static_cast<double>(allocation_counter::bytes) /
			static_cast<double>(iterations)
	};
}

result_t measure_dwt(
	const signal_t &signal,
	waveletpp::filter_t filter,
	bool extend,
	std::size_t iterations
)
{
	waveletpp::transform transform(signal, filter);
	return measure(
		"DWT", waveletpp::filter_name(filter), extend ? "extended" : "periodic",
		1, iterations,
		[&transform, extend] {
			const auto &result = transform.dwt(extend);
			return result.low[result.low.size() / 2] +
			       result.high[result.high.size() / 2];
		}
	);
}

result_t measure_construction(
	const signal_t &signal,
	waveletpp::filter_t filter,
	std::size_t iterations
)
{
	return measure(
		"Construct", waveletpp::filter_name(filter), "copy-input", 0, iterations,
		[&signal, filter] {
			waveletpp::transform transform(signal, filter);
			return transform.data().src[transform.data().src.size() / 2];
		}
	);
}

result_t measure_idwt(
	const signal_t &signal,
	waveletpp::filter_t filter,
	bool extend,
	std::size_t iterations
)
{
	waveletpp::transform transform(signal, filter);
	static_cast<void>(transform.dwt(extend));
	return measure(
		"IDWT", waveletpp::filter_name(filter), extend ? "extended" : "periodic",
		1, iterations,
		[&transform, extend] {
			const auto &result = transform.idwt(extend);
			return result[result.size() / 2];
		}
	);
}

result_t measure_lpf(
	const signal_t &signal,
	waveletpp::filter_t filter,
	std::size_t level,
	std::size_t iterations
)
{
	waveletpp::transform transform(signal, filter);
	return measure(
		"LPF", waveletpp::filter_name(filter), "extended", level, iterations,
		[&transform, level] {
			const auto &result = transform.lpf({0.1, level});
			return result[result.size() / 2];
		}
	);
}

void print_header(std::size_t sample_count, std::size_t iterations)
{
	std::cout << "Waveletpp transform performance\n"
	          << "  samples:    " << sample_count << '\n'
	          << "  iterations: " << iterations << '\n'
	          << "  clock:      steady_clock\n\n"
	          << std::left
	          << std::setw(10) << "operation"
	          << std::setw(10) << "filter"
	          << std::setw(12) << "mode"
	          << std::right
	          << std::setw(7)  << "level"
	          << std::setw(14) << "ns/op"
	          << std::setw(13) << "ns/sample"
	          << std::setw(13) << "Msample/s"
	          << std::setw(12) << "alloc/op"
	          << std::setw(13) << "KiB/op"
	          << '\n';
}

void print_result(const result_t &result, std::size_t sample_count)
{
	const auto nanoseconds_per_sample = result.nanoseconds /
	                                    static_cast<double>(sample_count);
	const auto million_samples_per_second = static_cast<double>(sample_count) *
	                                          1000.0 / result.nanoseconds;

	std::cout << std::left
	          << std::setw(10) << result.operation
	          << std::setw(10) << result.filter
	          << std::setw(12) << result.mode
	          << std::right
	          << std::setw(7)  << result.level
	          << std::fixed << std::setprecision(1)
	          << std::setw(14) << result.nanoseconds
	          << std::setw(13) << nanoseconds_per_sample
	          << std::setw(13) << million_samples_per_second
	          << std::setw(12) << result.allocations
	          << std::setw(13) << result.allocated_bytes / 1024.0
	          << '\n';
}

} // namespace

int main(int argc, char *argv[]) try
{
	if( argc > 3 )
	{
		std::cerr << "Usage: " << argv[0] << " [sample-count] [iterations]\n";
		return 2;
	}

	const auto sample_count = argc >= 2 ?
		parse_positive(argv[1], "sample-count") : default_sample_count;
	const auto iterations = argc == 3 ?
		parse_positive(argv[2], "iterations") : default_iterations;

	if( sample_count < minimum_sample_count or sample_count % 2 != 0 )
		throw std::invalid_argument("sample-count must be an even integer of at least 64");

	const auto signal = make_signal(sample_count);
	print_header(sample_count, iterations);

	print_result(measure_construction(
		signal, waveletpp::filter::coif5, iterations
	), sample_count);
	print_result(measure_dwt(
		signal, waveletpp::filter::db1, false, iterations
	), sample_count);
	print_result(measure_dwt(
		signal, waveletpp::filter::db4, false, iterations
	), sample_count);
	print_result(measure_dwt(
		signal, waveletpp::filter::coif5, false, iterations
	), sample_count);
	print_result(measure_dwt(
		signal, waveletpp::filter::coif5, true, iterations
	), sample_count);
	print_result(measure_idwt(
		signal, waveletpp::filter::db1, false, iterations
	), sample_count);
	print_result(measure_idwt(
		signal, waveletpp::filter::coif5, false, iterations
	), sample_count);
	print_result(measure_idwt(
		signal, waveletpp::filter::coif5, true, iterations
	), sample_count);
	print_result(measure_lpf(
		signal, waveletpp::filter::coif5, 8, iterations
	), sample_count);

	std::cout << "\nchecksum: " << checksum << '\n';
	return 0;
}
catch(const std::exception &error)
{
	allocation_counter::enabled = false;
	std::cerr << "error: " << error.what() << '\n';
	return 1;
}
