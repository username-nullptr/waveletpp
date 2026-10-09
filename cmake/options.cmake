# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

set(waveletpp_build_examples_default OFF)

if (DEFINED BUILD_EXAMPLES)
	set(waveletpp_build_examples_default ${BUILD_EXAMPLES})
	message(DEPRECATION
		"${PRO_NAME}: BUILD_EXAMPLES is deprecated; use "
		"WAVELETPP_BUILD_EXAMPLES instead."
	)
endif ()

option(WAVELETPP_BUILD_EXAMPLES
	"-- ${PRO_NAME}: Enable this to build the examples."
	${waveletpp_build_examples_default}
)
option(WAVELETPP_ENABLE_FAST_MATH
	"-- ${PRO_NAME}: Enable non-strict floating-point optimizations for consumers." OFF
)
if (WAVELETPP_BUILD_EXAMPLES)
	message(STATUS "${PRO_NAME}: Enable this to build the examples.")
endif ()
if (WAVELETPP_ENABLE_FAST_MATH)
	if (NOT MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
		message(FATAL_ERROR
			"${PRO_NAME}: Fast math requires MSVC, GCC, Clang, or AppleClang."
		)
	endif ()
	message(STATUS "${PRO_NAME}: Enable non-strict floating-point optimizations.")
endif ()

unset(waveletpp_build_examples_default)
