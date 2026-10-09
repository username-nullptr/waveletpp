# SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

set(WAVELETPP_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_COMPILER_IS_GNUCXX)
	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag("-Wa,-mbig-obj" WAVELETPP_TEST_GNU_BIG_OBJ)
endif ()


function(add_waveletpp_test_program test_target source)
	add_executable(${test_target} ${source})
	waveletpp_enable_strict_warnings(${test_target})
	target_link_libraries(${test_target} PRIVATE waveletpp::waveletpp)

	target_include_directories(${test_target} PRIVATE ${WAVELETPP_TEST_SOURCE_DIR})
	target_compile_options(${test_target} PRIVATE
		$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${WAVELETPP_TEST_GNU_BIG_OBJ}>>:-Wa,-mbig-obj>
	)
	if (WAVELETPP_ENABLE_TEST_SANITIZERS)
		target_link_libraries(${test_target} PRIVATE waveletpp.test.sanitizer)
	endif ()

	set_target_properties(${test_target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${WAVELETPP_OUTPUT_DIR}/test"
	)
endfunction()


function(add_waveletpp_functional_test name source)
	set(test_target waveletpp.test.${name})
	add_waveletpp_test_program(${test_target} ${source})

	add_test(NAME waveletpp.${name} COMMAND $<TARGET_FILE:${test_target}>)
	set_tests_properties(waveletpp.${name} PROPERTIES
		LABELS functional
		TIMEOUT ${WAVELETPP_FUNCTIONAL_TIMEOUT}
	)
	if (WAVELETPP_ENABLE_TEST_SANITIZERS)
		set_property(TEST waveletpp.${name} APPEND PROPERTY LABELS sanitizer)
		set_property(TEST waveletpp.${name} APPEND PROPERTY
			FAIL_REGULAR_EXPRESSION
			"AddressSanitizer" "LeakSanitizer" "runtime error:"
		)
		if (NOT MSVC)
			set_property(TEST waveletpp.${name} APPEND PROPERTY ENVIRONMENT
				"ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:abort_on_error=1"
				"UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1"
			)
		endif ()
	endif ()
endfunction()

add_subdirectory(functional)

if (WAVELETPP_BUILD_CMAKE_TESTS)
	add_subdirectory(cmake)
endif ()
