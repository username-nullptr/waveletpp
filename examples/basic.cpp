// SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#include <waveletpp.hpp>
#include <iostream>

int main()
{
	const std::vector signal {
		1.0, 1.2, 0.8, 1.1, 4.0, 4.2, 3.8, 4.1,
		1.0, 1.1, 0.9, 1.2, 4.1, 3.9, 4.2, 4.0
	};
	waveletpp::transform wavelet {
		signal, waveletpp::filter::db2
	};
	const auto &filtered = wavelet.lpf({0.25, 2});

	for(const auto value : filtered)
		std::cout << value << '\n';
	return 0;
}
