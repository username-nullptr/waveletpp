# SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

set(performance_target waveletpp.test.performance.transform)

add_executable(${performance_target} transform.cpp)
waveletpp_enable_strict_warnings(${performance_target})
target_link_libraries(${performance_target} PRIVATE waveletpp::waveletpp)

target_compile_options(${performance_target} PRIVATE
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${WAVELETPP_TEST_GNU_BIG_OBJ}>>:-Wa,-mbig-obj>
)
set_target_properties(${performance_target} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${WAVELETPP_OUTPUT_DIR}/test/performance"
)
add_custom_target(waveletpp.performance
	COMMAND $<TARGET_FILE:${performance_target}>
	DEPENDS ${performance_target}
	COMMENT "Running Waveletpp performance benchmarks"
	USES_TERMINAL
)
if (NOT CMAKE_CONFIGURATION_TYPES AND
	NOT CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo)$")
	message(WARNING
		"${PRO_NAME}: Performance results are most useful with "
		"CMAKE_BUILD_TYPE=Release or RelWithDebInfo."
	)
endif ()
