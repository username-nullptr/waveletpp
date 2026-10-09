// SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include <waveletpp.hpp>

#include <string_view>
#include <vector>

int main()
{
	if(std::string_view(waveletpp::filter_name(waveletpp::filter::db2)) != "db2")
		return 1;
	if(waveletpp::lo_d_size(waveletpp::filter::db2) != 4 ||
	   waveletpp::hi_d_size(waveletpp::filter::db2) != 4 ||
	   waveletpp::lo_r_size(waveletpp::filter::db2) != 4 ||
	   waveletpp::hi_r_size(waveletpp::filter::db2) != 4)
		return 2;

	const std::vector<double> signal {1.0, 2.0, 3.0, 4.0};
	waveletpp::transform transform(signal);
	return transform.dwt(false).low.size() == signal.size() / 2 ? 0 : 3;
}
