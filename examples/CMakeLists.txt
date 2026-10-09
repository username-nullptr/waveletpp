# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

if (CMAKE_COMPILER_IS_GNUCXX)
	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag("-Wa,-mbig-obj" WAVELETPP_EXAMPLE_GNU_BIG_OBJ)
endif ()

set(target_name waveletpp.example)
add_executable(${target_name} main.cpp)
waveletpp_enable_strict_warnings(${target_name})

target_link_libraries(${target_name} PRIVATE waveletpp::waveletpp)
target_compile_options(${target_name} PRIVATE
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${WAVELETPP_EXAMPLE_GNU_BIG_OBJ}>>:-Wa,-mbig-obj>
)
set_target_properties(${target_name} PROPERTIES
	OUTPUT_NAME example
	RUNTIME_OUTPUT_DIRECTORY "${WAVELETPP_OUTPUT_DIR}/examples"
)
install(TARGETS ${target_name}
	RUNTIME DESTINATION examples
)
