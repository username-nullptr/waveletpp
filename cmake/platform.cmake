# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

function(waveletpp_legacy_option new_name old_name description)
	set(default_value OFF)

	if (DEFINED ${old_name})
		set(default_value ${${old_name}})
		message(DEPRECATION
			"${PRO_NAME}: ${old_name} is deprecated; use ${new_name} instead."
		)
	endif ()

	option(${new_name} "${description}" ${default_value})
endfunction()


waveletpp_legacy_option(WAVELETPP_USE_LIBCXX USE_LIBCXX
	"-- ${PRO_NAME}: Use Clang libc++."
)
waveletpp_legacy_option(WAVELETPP_USE_LLD USE_LLD
	"-- ${PRO_NAME}: Use Clang lld."
)
waveletpp_legacy_option(WAVELETPP_ENABLE_LTO ENABLE_LTO
	"-- ${PRO_NAME}: Enable link-time optimization."
)
if (WAVELETPP_USE_LIBCXX AND NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
	message(FATAL_ERROR
		"${PRO_NAME}: WAVELETPP_USE_LIBCXX requires a Clang compiler."
	)
endif ()

if (WAVELETPP_USE_LLD AND NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
	message(FATAL_ERROR
		"${PRO_NAME}: WAVELETPP_USE_LLD requires a Clang compiler."
	)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
		message(FATAL_ERROR "The minimum version of 'Clang' required is 5.")
	endif ()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
		message(FATAL_ERROR "The minimum version of 'AppleClang' required is 9.")
	endif ()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
		message(FATAL_ERROR "The minimum version of 'GNU' required is 7.")
	endif ()

elseif (MSVC)
	if (MSVC_VERSION LESS 1910)
		message(FATAL_ERROR "The minimum version of 'MSVC' required is 1910 (VS2017).")
	endif ()

else ()
	message(STATUS
		"${PRO_NAME}: Unknown compiler: ${CMAKE_CXX_COMPILER_ID} "
		"(${CMAKE_CXX_COMPILER_VERSION})."
	)
endif ()

if (WAVELETPP_USE_LIBCXX)
	message(STATUS "${PRO_NAME}: Use Clang libc++.")
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
	add_link_options(-stdlib=libc++)
endif ()

if (WAVELETPP_USE_LLD)
	message(STATUS "${PRO_NAME}: Use Clang lld.")
	add_link_options(-fuse-ld=lld)
endif ()

if (WAVELETPP_ENABLE_LTO)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT waveletpp_lto_available
		OUTPUT waveletpp_lto_error LANGUAGES CXX
	)
	if (NOT waveletpp_lto_available)
		message(FATAL_ERROR
			"${PRO_NAME}: Link-time optimization is unavailable: "
			"${waveletpp_lto_error}"
		)
	endif ()

	message(STATUS "${PRO_NAME}: Enable link-time optimization.")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif ()

if (MSVC)
	add_compile_options(/Zc:__cplusplus /Zc:preprocessor)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(WAVELETPP_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output)

message(STATUS "")
message(STATUS "${PRO_NAME}: Using C++: ${CMAKE_CXX_STANDARD}")
message(STATUS "${PRO_NAME}: Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "${PRO_NAME}: Install prefix: ${CMAKE_INSTALL_PREFIX}")
