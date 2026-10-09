// SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include <waveletpp.hpp>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <iomanip>

namespace
{

using signal_t = std::vector<double>;

constexpr std::size_t sample_count = 8192;
constexpr double sample_rate = 1024.0;
constexpr double pi = 3.14159265358979323846;

struct signals_t
{
	signal_t clean;
	signal_t noisy;
};

signals_t make_signals()
{
	signals_t signals;
	signals.clean.reserve(sample_count);
	signals.noisy.reserve(sample_count);

	std::uint32_t random_state = 0x12345678U;
	for(std::size_t i=0; i<sample_count; ++i)
	{
		const auto time = static_cast<double>(i) / sample_rate;
		const auto clean = std::sin(2.0 * pi * 5.0 * time) + 0.35 * std::sin(2.0 * pi * 13.0 * time);

		// A fixed pseudo-random sequence keeps every run reproducible.
		random_state = random_state * 1664525U + 1013904223U;

		const auto random_noise = 2.0 * static_cast<double>(random_state >> 8U) / 16777215.0 - 1.0;
		const auto noise = 0.25 * random_noise + 0.12 * std::sin(2.0 * pi * 180.0 * time);

		signals.clean.emplace_back(clean);
		signals.noisy.emplace_back(clean + noise);
	}
	return signals;
}

void write_csv
(const std::string &file_name, const signal_t &clean, const signal_t &noisy, const signal_t &filtered)
{
	if( clean.size() != noisy.size() || clean.size() != filtered.size() )
		throw std::runtime_error("signals must have the same length");

	std::ofstream output(file_name);
	if( not output )
		throw std::runtime_error("cannot open output file: " + file_name);

	output << "sample,time_seconds,clean,noisy,filtered\n"
	       << std::fixed << std::setprecision(10);

	for(std::size_t i=0; i<clean.size(); ++i)
	{
		output << i << ','
		       << static_cast<double>(i) / sample_rate << ','
		       << clean[i] << ','
		       << noisy[i] << ','
		       << filtered[i] << '\n';
	}
	if( not output )
		throw std::runtime_error("failed to write output file: " + file_name);
}

} // namespace

int main(int argc, char *argv[]) try
{
	if( argc > 2 )
	{
		std::cerr << "Usage: " << argv[0] << " [output.csv]\n";
		return 2;
	}
	const std::string file_name = argc == 2 ? argv[1] : "wavelet_signal.csv";
	const auto signals = make_signals();

	waveletpp::transform wavelet(signals.noisy, waveletpp::filter::db4);
	const auto filtered = wavelet.lpf({0.30, 4});
	write_csv(file_name, signals.clean, signals.noisy, filtered);

	std::cout << "Wrote " << sample_count << " samples to " << file_name << "\n"
	          << "Open the CSV in Office or WPS and plot time_seconds with "
	             "clean, noisy, and filtered.\n";
	return 0;
}
catch(const std::exception &error)
{
	std::cerr << "error: " << error.what() << '\n';
	return 1;
}
