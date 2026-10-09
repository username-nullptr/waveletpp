// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include "test.h"
#include "reference_coefficients.h"

#include <waveletpp/filter.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace
{

struct filter_case
{
	waveletpp::filter value;
	std::string_view name;
	std::wstring_view wide_name;
};

constexpr filter_case filters[] {
#define X_MACRO(value, name) {waveletpp::filter::value, name, L##name},
	WAVELETPP_FILTER_LIST
#undef X_MACRO
};

std::size_t expected_filter_size(waveletpp::filter value)
{
	switch(value)
	{
	case waveletpp::filter::db1: return 2;
	case waveletpp::filter::db2: return 4;
	case waveletpp::filter::db3: return 6;
	case waveletpp::filter::db4: return 8;
	case waveletpp::filter::db5: return 10;
	case waveletpp::filter::db6: return 12;
	case waveletpp::filter::db7: return 14;
	case waveletpp::filter::db8: return 16;
	case waveletpp::filter::db9: return 18;
	case waveletpp::filter::db10: return 20;
	case waveletpp::filter::db11: return 22;
	case waveletpp::filter::db12: return 24;
	case waveletpp::filter::db13: return 26;
	case waveletpp::filter::db14: return 28;
	case waveletpp::filter::db15: return 30;
	case waveletpp::filter::bior11: return 2;
	case waveletpp::filter::bior13: return 6;
	case waveletpp::filter::bior15: return 10;
	case waveletpp::filter::bior22: return 6;
	case waveletpp::filter::bior24: return 10;
	case waveletpp::filter::bior26: return 14;
	case waveletpp::filter::bior28: return 18;
	case waveletpp::filter::bior31: return 4;
	case waveletpp::filter::bior33: return 8;
	case waveletpp::filter::bior35: return 12;
	case waveletpp::filter::bior37: return 16;
	case waveletpp::filter::bior39: return 20;
	case waveletpp::filter::bior44: return 10;
	case waveletpp::filter::bior55: return 12;
	case waveletpp::filter::bior68: return 18;
	case waveletpp::filter::coif1: return 6;
	case waveletpp::filter::coif2: return 12;
	case waveletpp::filter::coif3: return 18;
	case waveletpp::filter::coif4: return 24;
	case waveletpp::filter::coif5: return 30;
	case waveletpp::filter::sym2: return 4;
	case waveletpp::filter::sym3: return 6;
	case waveletpp::filter::sym4: return 8;
	case waveletpp::filter::sym5: return 10;
	case waveletpp::filter::sym6: return 12;
	case waveletpp::filter::sym7: return 14;
	case waveletpp::filter::sym8: return 16;
	case waveletpp::filter::sym9: return 18;
	case waveletpp::filter::sym10: return 20;
	}
	throw std::invalid_argument("unknown filter enum");
}

double sum(const std::vector<double> &values)
{
	double result = 0.0;
	for(const auto value : values)
		result += value;
	return result;
}

void check_near(double actual, double expected, double tolerance, const std::string &context)
{
	if(std::abs(actual - expected) <= tolerance)
		return;
	std::ostringstream message;
	message.precision(17);
	message << context << ": expected " << expected << ", got " << actual
	        << " (tolerance " << tolerance << ')';
	throw std::runtime_error(message.str());
}

void names_round_trip()
{
	for(const auto &entry : filters)
	{
		WAVELETPP_TEST_CHECK_EQ(
			std::string_view(waveletpp::filter_name(entry.value)), entry.name);
		WAVELETPP_TEST_CHECK_EQ(
			std::wstring_view(waveletpp::wfilter_name(entry.value)), entry.wide_name);
		WAVELETPP_TEST_CHECK_EQ(
			waveletpp::from_filter_name(entry.name), entry.value);
		WAVELETPP_TEST_CHECK_EQ(
			waveletpp::from_filter_name(entry.wide_name), entry.value);
		WAVELETPP_TEST_CHECK_EQ(
			waveletpp::filter_arg_enum(entry.name), entry.value);
		WAVELETPP_TEST_CHECK_EQ(
			waveletpp::filter_arg_name(entry.value), std::string(entry.name));
		WAVELETPP_TEST_CHECK_EQ(
			waveletpp::filter_arg_wname(entry.value), std::wstring(entry.wide_name));
		WAVELETPP_TEST_CHECK(waveletpp::check_filter(entry.value));
		WAVELETPP_TEST_CHECK(waveletpp::check_filter(entry.name));
		WAVELETPP_TEST_CHECK(waveletpp::check_filter(entry.wide_name));
	}

	WAVELETPP_TEST_CHECK_EQ(
		std::string_view(waveletpp::filter_name<waveletpp::filter::db4>()),
		std::string_view("db4"));
	WAVELETPP_TEST_CHECK_EQ(
		std::wstring_view(waveletpp::wfilter_name<waveletpp::filter::sym5>()),
		std::wstring_view(L"sym5"));
}

void coefficient_tables_are_complete()
{
	for(const auto &entry : filters)
	{
		const auto low_decomposition = waveletpp::lo_d(entry.value);
		const auto high_decomposition = waveletpp::hi_d(entry.value);
		const auto low_reconstruction = waveletpp::lo_r(entry.value);
		const auto high_reconstruction = waveletpp::hi_r(entry.value);

		WAVELETPP_TEST_CHECK(not low_decomposition.empty());
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size(), expected_filter_size(entry.value));
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size(), high_decomposition.size());
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size(), low_reconstruction.size());
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size(), high_reconstruction.size());
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size() % 2, std::size_t {0});
		WAVELETPP_TEST_CHECK_EQ(waveletpp::lo_d_size(entry.value), low_decomposition.size());
		WAVELETPP_TEST_CHECK_EQ(waveletpp::hi_d_size(entry.value), high_decomposition.size());
		WAVELETPP_TEST_CHECK_EQ(waveletpp::lo_r_size(entry.value), low_reconstruction.size());
		WAVELETPP_TEST_CHECK_EQ(waveletpp::hi_r_size(entry.value), high_reconstruction.size());

		for(const auto *coefficients : {
			&low_decomposition, &high_decomposition,
			&low_reconstruction, &high_reconstruction
		})
		{
			WAVELETPP_TEST_CHECK(std::all_of(
				coefficients->begin(), coefficients->end(),
				[](double value) { return std::isfinite(value); }
			));
		}
	}
}

void haar_coefficients_have_full_double_precision()
{
	const auto expected = std::sqrt(0.5);
	const auto tolerance = 2.0 * std::numeric_limits<double>::epsilon();
	for(const auto value : waveletpp::lo_d(waveletpp::filter::db1))
		check_near(value, expected, tolerance, "db1 decomposition low-pass coefficient");
	for(const auto value : waveletpp::lo_r(waveletpp::filter::db1))
		check_near(value, expected, tolerance, "db1 reconstruction low-pass coefficient");
}

bool matches_reference_up_to_phase_and_sign(
	const std::vector<double> &actual,
	const std::vector<double> &reference,
	double tolerance
)
{
	if(actual.size() != reference.size())
		return false;
	for(const auto reversed : {false, true})
	{
		for(const auto sign : {-1.0, 1.0})
		{
			bool matches = true;
			for(std::size_t index = 0; index < actual.size(); ++index)
			{
				const auto reference_index = reversed ? actual.size() - 1 - index : index;
				const auto expected = sign * reference[reference_index];
				const auto scale = std::max({1.0, std::abs(actual[index]), std::abs(expected)});
				if(std::abs(actual[index] - expected) > tolerance * scale)
				{
					matches = false;
					break;
				}
			}
			if(matches)
				return true;
		}
	}
	return false;
}

void coefficient_values_match_independent_reference()
{
	constexpr double tolerance = 32.0 * std::numeric_limits<double>::epsilon();
	const auto references = waveletpp::test::reference_filter_banks();
	WAVELETPP_TEST_CHECK_EQ(references.size(), std::size(filters));

	for(const auto &reference : references)
	{
		const std::array actual {
			waveletpp::lo_d(reference.value),
			waveletpp::lo_r(reference.value)
		};
		const std::array<const std::vector<double> *,2> expected {
			&reference.decomposition_low,
			&reference.reconstruction_low
		};
		std::array<bool,2> used {};

		for(std::size_t actual_index = 0; actual_index < actual.size(); ++actual_index)
		{
			bool found = false;
			for(std::size_t expected_index = 0; expected_index < expected.size(); ++expected_index)
			{
				if(used[expected_index] || not matches_reference_up_to_phase_and_sign(
					actual[actual_index], *expected[expected_index], tolerance))
					continue;
				used[expected_index] = true;
				found = true;
				break;
			}
			if(not found)
			{
				throw std::runtime_error(
					std::string(reference.name) +
					" coefficient bank differs from the independent reference");
			}
		}
	}
}

void coefficient_banks_satisfy_normalization_and_qmf_relations()
{
	constexpr double normalization_tolerance = 1e-11;
	constexpr double relation_tolerance = 8.0 * std::numeric_limits<double>::epsilon();
	const auto sqrt_two = std::sqrt(2.0);

	for(const auto &entry : filters)
	{
		const auto low_decomposition = waveletpp::lo_d(entry.value);
		const auto high_decomposition = waveletpp::hi_d(entry.value);
		const auto low_reconstruction = waveletpp::lo_r(entry.value);
		const auto high_reconstruction = waveletpp::hi_r(entry.value);
		const auto context = std::string(entry.name);

		check_near(sum(low_decomposition), sqrt_two, normalization_tolerance,
			context + " decomposition low-pass DC gain");
		check_near(sum(low_reconstruction), sqrt_two, normalization_tolerance,
			context + " reconstruction low-pass DC gain");
		check_near(sum(high_decomposition), 0.0, normalization_tolerance,
			context + " decomposition high-pass DC rejection");
		check_near(sum(high_reconstruction), 0.0, normalization_tolerance,
			context + " reconstruction high-pass DC rejection");

		for(std::size_t index = 0; index < low_decomposition.size(); ++index)
		{
			const auto decomposition_sign = index % 2 == 0 ? 1.0 : -1.0;
			const auto reconstruction_sign = -decomposition_sign;
			check_near(
				high_decomposition[index], decomposition_sign * low_reconstruction[index],
				relation_tolerance, context + " decomposition QMF relation");
			check_near(
				high_reconstruction[index], reconstruction_sign * low_decomposition[index],
				relation_tolerance, context + " reconstruction QMF relation");
		}

		if(entry.name.substr(0, 4) != "bior")
		{
			for(std::size_t shift = 0; shift < low_decomposition.size() / 2; ++shift)
			{
				double correlation = 0.0;
				for(std::size_t index = 0;
					index + 2 * shift < low_decomposition.size(); ++index)
				{
					correlation += low_decomposition[index] *
						low_decomposition[index + 2 * shift];
				}
				check_near(correlation, shift == 0 ? 1.0 : 0.0,
					normalization_tolerance,
					context + " even-shift orthogonality");
			}
		}
	}
}

void invalid_filters_are_rejected()
{
	const auto invalid_filter = static_cast<waveletpp::filter>(
		static_cast<int>(filters[std::size(filters) - 1].value) + 1);
	WAVELETPP_TEST_CHECK(not waveletpp::check_filter("not-a-filter"));
	WAVELETPP_TEST_CHECK(not waveletpp::check_filter(""));
	WAVELETPP_TEST_CHECK(not waveletpp::check_filter("DB4"));
	WAVELETPP_TEST_CHECK(not waveletpp::check_filter(L"not-a-filter"));
	WAVELETPP_TEST_CHECK(not waveletpp::check_filter(invalid_filter));

	bool threw = false;
	try
	{
		static_cast<void>(waveletpp::from_filter_name("not-a-filter"));
	}
	catch(const std::invalid_argument &)
	{
		threw = true;
	}
	WAVELETPP_TEST_CHECK(threw);

	threw = false;
	try
	{
		static_cast<void>(waveletpp::filter_name(
			static_cast<waveletpp::filter>(-1)));
	}
	catch(const std::invalid_argument &)
	{
		threw = true;
	}
	WAVELETPP_TEST_CHECK(threw);

	threw = false;
	try
	{
		static_cast<void>(waveletpp::check_filter("not-a-filter", true));
	}
	catch(const std::invalid_argument &)
	{
		threw = true;
	}
	WAVELETPP_TEST_CHECK(threw);

	for(const auto getter : {
		+[](waveletpp::filter value) { static_cast<void>(waveletpp::lo_d(value)); },
		+[](waveletpp::filter value) { static_cast<void>(waveletpp::hi_d(value)); },
		+[](waveletpp::filter value) { static_cast<void>(waveletpp::lo_r(value)); },
		+[](waveletpp::filter value) { static_cast<void>(waveletpp::hi_r(value)); }
	})
	{
		threw = false;
		try
		{
			getter(invalid_filter);
		}
		catch(const std::invalid_argument &)
		{
			threw = true;
		}
		WAVELETPP_TEST_CHECK(threw);
	}
}

} // namespace

int main()
{
	return waveletpp::test::run({
		{"filter names round trip", names_round_trip},
		{"coefficient tables are complete", coefficient_tables_are_complete},
		{"coefficient values match an independent reference", coefficient_values_match_independent_reference},
		{"Haar coefficients retain full precision", haar_coefficients_have_full_double_precision},
		{"coefficient banks satisfy normalization and QMF relations", coefficient_banks_satisfy_normalization_and_qmf_relations},
		{"invalid filters are rejected", invalid_filters_are_rejected},
	});
}
