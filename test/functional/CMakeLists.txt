# SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

add_waveletpp_functional_test(filter filter.cpp)
add_waveletpp_functional_test(transform transform.cpp)
add_waveletpp_functional_test(numeric numeric.cpp)

if (WAVELETPP_ENABLE_TEST_SANITIZERS)
	add_waveletpp_functional_test(safety safety.cpp)
endif ()
