# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

option(WAVELETPP_STRICT_WARNINGS
	"-- ${PRO_NAME}: Enable strict warnings for first-party compiled targets." ON
)
option(WAVELETPP_WARNINGS_AS_ERRORS
	"-- ${PRO_NAME}: Treat first-party warnings as errors." OFF
)


function(waveletpp_enable_strict_warnings target)
	if (NOT TARGET ${target})
		message(FATAL_ERROR
			"${PRO_NAME}: Cannot enable warnings for missing target '${target}'."
		)
	endif ()

	if (NOT WAVELETPP_STRICT_WARNINGS)
		return()
	endif ()

	if (MSVC)
		target_compile_options(${target} PRIVATE
			/W4
			/permissive-
			/Zc:__cplusplus
			/Zc:preprocessor
			/utf-8
		)
		if (WAVELETPP_WARNINGS_AS_ERRORS)
			target_compile_options(${target} PRIVATE /WX)
		endif ()

	elseif (CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
		target_compile_options(${target} PRIVATE
			-Wall
			-Wextra
			-Wpedantic
			-Wformat=2
			-Wundef
		)
		if (WAVELETPP_WARNINGS_AS_ERRORS)
			target_compile_options(${target} PRIVATE -Werror)
		endif ()
	endif ()
endfunction()
