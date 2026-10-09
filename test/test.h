// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#ifndef WAVELETPP_TEST_TEST_H
#define WAVELETPP_TEST_TEST_H

#include <exception>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace waveletpp::test
{

[[noreturn]] inline void fail(
	std::string_view expression, const char *file, int line
)
{
	throw std::runtime_error(
		std::string(file) + ':' + std::to_string(line) +
		": check failed: " + std::string(expression)
	);
}

inline void check(
	bool condition, std::string_view expression, const char *file, int line
)
{
	if (not condition)
		fail(expression, file, line);
}

template <typename Actual, typename Expected>
inline void check_equal(
	const Actual &actual,
	const Expected &expected,
	std::string_view actual_expression,
	std::string_view expected_expression,
	const char *file,
	int line
)
{
	if (actual == expected)
		return;

	fail(std::string(actual_expression) + " == " +
		std::string(expected_expression), file, line);
}

struct test_case
{
	std::string_view name;
	std::function<void()> function;
};

inline int run(std::initializer_list<test_case> tests)
{
	std::size_t passed = 0;
	for(const auto &test : tests)
	{
		try
		{
			test.function();
			++passed;
			std::cout << "[PASS] " << test.name << '\n';
		}
		catch(const std::exception &error)
		{
			std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
			return 1;
		}
		catch(...)
		{
			std::cerr << "[FAIL] " << test.name << ": unknown exception\n";
			return 1;
		}
	}
	std::cout << passed << " test cases passed\n";
	return 0;
}

} // namespace waveletpp::test

#define WAVELETPP_TEST_CHECK(expression) \
	::waveletpp::test::check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
#define WAVELETPP_TEST_CHECK_EQ(actual, expected) \
	::waveletpp::test::check_equal((actual), (expected), #actual, #expected, __FILE__, __LINE__)

#endif // WAVELETPP_TEST_TEST_H
