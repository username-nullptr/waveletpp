# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

set(waveletpp_build_examples_default ON)

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
if (WAVELETPP_BUILD_EXAMPLES)
	message(STATUS "${PRO_NAME}: Enable this to build the examples.")
endif ()

unset(waveletpp_build_examples_default)
