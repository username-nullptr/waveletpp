// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include "test.h"

#include <waveletpp.hpp>

#include <stdexcept>

namespace
{

void undersized_decomposition_is_rejected_before_boundary_extension()
{
	waveletpp::transform transform(waveletpp::filter::db4);
	transform.data().dec.low = {1.0};
	transform.data().dec.high = {0.5};

	bool threw = false;
	try
	{
		static_cast<void>(transform.idwt(true));
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
		{"undersized decomposition is rejected before extension", undersized_decomposition_is_rejected_before_boundary_extension},
	});
}
