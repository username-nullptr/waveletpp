// SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include <waveletpp.hpp>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace
{

using signal_t = std::vector<double>;

constexpr std::size_t sample_count = 128;
constexpr double pi = 3.14159265358979323846;

signal_t make_clean_signal()
{
	signal_t signal;
	signal.reserve(sample_count);

	for(std::size_t i=0; i<sample_count; ++i)
	{
		const auto phase = 2.0 * pi * static_cast<double>(i) / static_cast<double>(sample_count);
		signal.emplace_back(std::sin(phase) + 0.35 * std::sin(3.0 * phase));
	}
	return signal;
}

signal_t add_deterministic_noise(const signal_t &clean)
{
	constexpr double noise[] {
		 0.18, -0.12,  0.08, -0.20,  0.14, -0.06,  0.11, -0.16,
		-0.09,  0.17, -0.13,  0.07, -0.18,  0.10, -0.05,  0.15
	};
	signal_t noisy;
	noisy.reserve(clean.size());

	for(std::size_t i=0; i<clean.size(); ++i)
		noisy.emplace_back(clean[i] + noise[i % (sizeof(noise) / sizeof(noise[0]))]);
	return noisy;
}

double root_mean_square_error(const signal_t &actual, const signal_t &expected)
{
	if( actual.size() != expected.size() )
		throw std::runtime_error("cannot compare signals with different lengths");

	double squared_error = 0.0;
	for(std::size_t i=0; i<actual.size(); ++i)
	{
		const auto error = actual[i] - expected[i];
		squared_error += error * error;
	}
	return std::sqrt(squared_error / static_cast<double>(actual.size()));
}

void print_coefficients(const waveletpp::transform::decomposed_t &coefficients)
{
	constexpr std::size_t preview_size = 8;

	const auto count = std::min ({
		preview_size, coefficients.low.size(), coefficients.high.size()
	});
	std::cout << "\nOne-level DWT (first " << count << " coefficient pairs)\n"
	          << "  index       approximation             detail\n";

	for(std::size_t i=0; i<count; ++i)
	{
		std::cout << std::setw(7)  << i
		          << std::setw(20) << coefficients.low[i]
		          << std::setw(19) << coefficients.high[i]
				  << '\n';
	}
}

void print_samples(const signal_t &clean, const signal_t &noisy, const signal_t &filtered)
{
	constexpr std::size_t preview_size = 12;
	const auto count = std::min({preview_size, clean.size(), noisy.size(), filtered.size()});

	std::cout << "\nSignal preview (first " << count << " samples)\n"
	          << "  index         clean         noisy      filtered\n";

	for(std::size_t i=0; i<count; ++i)
	{
		std::cout << std::setw(7)  << i
		          << std::setw(14) << clean[i]
		          << std::setw(14) << noisy[i]
		          << std::setw(14) << filtered[i]
				  << '\n';
	}
}

} // namespace

int main(int argc, char *argv[]) try
{
	if( argc > 2 )
	{
		std::cerr << "Usage: " << argv[0] << " [filter-name]\n"
		          << "Example: " << argv[0] << " sym5\n";
		return 2;
	}
	const std::string_view requested_filter = argc == 2 ? argv[1] : "db4";
	const auto selected_filter = waveletpp::from_filter_name(requested_filter);

	const auto clean = make_clean_signal();
	const auto noisy = add_deterministic_noise(clean);

	std::cout << std::fixed << std::setprecision(6)
	          << "Wavelet++ advanced example\n"
	          << "  filter:     " << waveletpp::filter_name(selected_filter) << '\n'
	          << "  samples:    " << noisy.size() << '\n';

	// Transform operations reuse internal buffers, so retain results by value.
	waveletpp::transform decomposition(noisy, selected_filter);
	const auto coefficients = decomposition.dwt(false);

	print_coefficients(coefficients);
	const auto reconstructed = decomposition.idwt(false);

	constexpr waveletpp::transform::param_t denoise_parameters {0.25, 2};
	waveletpp::transform denoiser(noisy, selected_filter);

	const auto filtered = denoiser.lpf(denoise_parameters);
	print_samples(clean, noisy, filtered);

	std::cout << "\nQuality metrics (lower is better)\n"
	          << "  DWT/IDWT round-trip RMSE: "
	          << root_mean_square_error(reconstructed, noisy) << '\n'
	          << "  noisy signal RMSE:        "
	          << root_mean_square_error(noisy, clean) << '\n'
	          << "  filtered signal RMSE:     "
	          << root_mean_square_error(filtered, clean) << '\n'
	          << "  denoising parameters:     threshold=" << denoise_parameters.threshold
	          << ", levels=" << denoise_parameters.level << '\n';
	return 0;
}
catch(const std::exception &error)
{
	std::cerr << "error: " << error.what() << '\n';
	return 1;
}
