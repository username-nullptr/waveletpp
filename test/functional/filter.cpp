// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include "test.h"

#include <waveletpp/filter.h>

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
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size(), high_decomposition.size());
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size(), low_reconstruction.size());
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size(), high_reconstruction.size());
		WAVELETPP_TEST_CHECK_EQ(low_decomposition.size() % 2, std::size_t {0});
	}
}

void invalid_filters_are_rejected()
{
	WAVELETPP_TEST_CHECK(not waveletpp::check_filter("not-a-filter"));

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
}

} // namespace

int main()
{
	return waveletpp::test::run({
		{"filter names round trip", names_round_trip},
		{"coefficient tables are complete", coefficient_tables_are_complete},
		{"invalid filters are rejected", invalid_filters_are_rejected},
	});
}
