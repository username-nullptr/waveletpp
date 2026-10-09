# SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

option(BUILD_TESTING
	"-- ${PRO_NAME}: Build tests." OFF
)
option(WAVELETPP_BUILD_CMAKE_TESTS
	"-- ${PRO_NAME}: Test the installed CMake package." ${BUILD_TESTING}
)
option(WAVELETPP_ENABLE_TEST_SANITIZERS
	"-- ${PRO_NAME}: Enable AddressSanitizer and, where available, UndefinedBehaviorSanitizer for tests." OFF
)
option(WAVELETPP_BUILD_PERFORMANCE_TESTS
	"-- ${PRO_NAME}: Build the standalone performance benchmarks." OFF
)
set(WAVELETPP_FUNCTIONAL_TIMEOUT 30 CACHE STRING
	"CTest timeout in seconds for each Waveletpp functional executable."
)
set(WAVELETPP_CMAKE_TEST_TIMEOUT 60 CACHE STRING
	"CTest timeout in seconds for each Waveletpp CMake integration test."
)
foreach(option
	WAVELETPP_FUNCTIONAL_TIMEOUT
	WAVELETPP_CMAKE_TEST_TIMEOUT
)
	if (NOT ${option} MATCHES "^[1-9][0-9]*$")
		message(FATAL_ERROR "${option} must be a positive integer.")
	endif ()
endforeach()

if (WAVELETPP_BUILD_CMAKE_TESTS AND NOT BUILD_TESTING)
	message(FATAL_ERROR
		"${PRO_NAME}: CMake integration tests require BUILD_TESTING=ON."
	)
endif ()

if (WAVELETPP_BUILD_PERFORMANCE_TESTS AND NOT BUILD_TESTING)
	message(FATAL_ERROR
		"${PRO_NAME}: Performance benchmarks require BUILD_TESTING=ON."
	)
endif ()

if (WAVELETPP_ENABLE_TEST_SANITIZERS)
	if (NOT BUILD_TESTING)
		message(FATAL_ERROR
			"${PRO_NAME}: Test sanitizers require BUILD_TESTING=ON."
		)
	endif ()

	add_library(waveletpp.test.sanitizer INTERFACE)
	if (MSVC)
		target_compile_options(waveletpp.test.sanitizer INTERFACE
			/fsanitize=address
		)
		target_link_options(waveletpp.test.sanitizer INTERFACE
			/INCREMENTAL:NO
		)
	elseif (CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
		target_compile_options(waveletpp.test.sanitizer INTERFACE
			-fsanitize=address,undefined
			-fno-omit-frame-pointer
			-fno-sanitize-recover=all
		)
		target_link_options(waveletpp.test.sanitizer INTERFACE
			-fsanitize=address,undefined
		)
	else ()
		message(FATAL_ERROR
			"${PRO_NAME}: Test sanitizers require MSVC, GCC, Clang, or AppleClang."
		)
	endif ()
endif ()
