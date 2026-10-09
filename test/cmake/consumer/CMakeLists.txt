# SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(WaveletppConsumer LANGUAGES CXX)

find_package(Waveletpp 0.2 CONFIG REQUIRED)

if (NOT TARGET waveletpp::waveletpp)
	message(FATAL_ERROR "Waveletpp did not export waveletpp::waveletpp.")
endif ()

if (NOT Waveletpp_VERSION STREQUAL WAVELETPP_EXPECTED_VERSION)
	message(FATAL_ERROR
		"Expected Waveletpp ${WAVELETPP_EXPECTED_VERSION}, found ${Waveletpp_VERSION}."
	)
endif ()

add_executable(waveletpp_consumer main.cpp)
target_link_libraries(waveletpp_consumer PRIVATE waveletpp::waveletpp)

enable_testing()
add_test(NAME waveletpp_consumer COMMAND waveletpp_consumer)
