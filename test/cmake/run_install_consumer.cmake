# SPDX-FileCopyrightText: 2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

foreach(required_variable
	TEST_SOURCE_DIR
	TEST_BINARY_DIR
	TEST_INSTALL_PREFIX
	TEST_INSTALL_CMAKEDIR
	WAVELETPP_BINARY_DIR
	TEST_GENERATOR
	TEST_VERSION
	TEST_CTEST_COMMAND
)
	if (NOT DEFINED ${required_variable})
		message(FATAL_ERROR "Missing ${required_variable}.")
	endif ()
endforeach()

file(REMOVE_RECURSE "${TEST_BINARY_DIR}" "${TEST_INSTALL_PREFIX}")

set(install_command
	"${CMAKE_COMMAND}" --install "${WAVELETPP_BINARY_DIR}"
	--prefix "${TEST_INSTALL_PREFIX}"
)
if (TEST_CONFIGURATION)
	list(APPEND install_command --config "${TEST_CONFIGURATION}")
endif ()

execute_process (
	COMMAND ${install_command}
	RESULT_VARIABLE install_result
	OUTPUT_VARIABLE install_stdout
	ERROR_VARIABLE install_stderr
)
if (NOT install_result EQUAL 0)
	message(FATAL_ERROR
		"Installing Waveletpp failed.\n${install_stdout}\n${install_stderr}"
	)
endif ()

if (IS_ABSOLUTE "${TEST_INSTALL_CMAKEDIR}")
	set(waveletpp_package_dir "${TEST_INSTALL_CMAKEDIR}")
else ()
	set(waveletpp_package_dir
		"${TEST_INSTALL_PREFIX}/${TEST_INSTALL_CMAKEDIR}"
	)
endif ()

set(configure_command
	"${CMAKE_COMMAND}"
	-S "${TEST_SOURCE_DIR}"
	-B "${TEST_BINARY_DIR}"
	-G "${TEST_GENERATOR}"
)
if (TEST_GENERATOR_PLATFORM)
	list(APPEND configure_command -A "${TEST_GENERATOR_PLATFORM}")
endif ()

if (TEST_GENERATOR_TOOLSET)
	list(APPEND configure_command -T "${TEST_GENERATOR_TOOLSET}")
endif ()

if (TEST_MAKE_PROGRAM)
	list(APPEND configure_command "-DCMAKE_MAKE_PROGRAM=${TEST_MAKE_PROGRAM}")
endif ()

if (TEST_CXX_COMPILER)
	list(APPEND configure_command "-DCMAKE_CXX_COMPILER=${TEST_CXX_COMPILER}")
endif ()

if (TEST_TOOLCHAIN_FILE)
	list(APPEND configure_command "-DCMAKE_TOOLCHAIN_FILE=${TEST_TOOLCHAIN_FILE}")
endif ()

if (TEST_BUILD_TYPE)
	list(APPEND configure_command "-DCMAKE_BUILD_TYPE=${TEST_BUILD_TYPE}")
endif ()

list(APPEND configure_command
	"-DWaveletpp_DIR=${waveletpp_package_dir}"
	"-DWAVELETPP_EXPECTED_VERSION=${TEST_VERSION}"
)

execute_process(
	COMMAND ${configure_command}
	RESULT_VARIABLE configure_result
	OUTPUT_VARIABLE configure_stdout
	ERROR_VARIABLE configure_stderr
)
if (NOT configure_result EQUAL 0)
	message(FATAL_ERROR
		"Configuring the installed-package consumer failed.\n"
		"${configure_stdout}\n${configure_stderr}"
	)
endif ()

set(build_command "${CMAKE_COMMAND}" --build "${TEST_BINARY_DIR}" --parallel 2)
if (TEST_CONFIGURATION)
	list(APPEND build_command --config "${TEST_CONFIGURATION}")
endif ()
execute_process(
	COMMAND ${build_command}
	RESULT_VARIABLE build_result
	OUTPUT_VARIABLE build_stdout
	ERROR_VARIABLE build_stderr
)
if (NOT build_result EQUAL 0)
	message(FATAL_ERROR
		"Building the installed-package consumer failed.\n"
		"${build_stdout}\n${build_stderr}"
	)
endif ()

set(test_command "${TEST_CTEST_COMMAND}" --output-on-failure)
if (TEST_CONFIGURATION)
	list(APPEND test_command -C "${TEST_CONFIGURATION}")
endif ()
execute_process(
	COMMAND ${test_command}
	WORKING_DIRECTORY "${TEST_BINARY_DIR}"
	RESULT_VARIABLE test_result
	OUTPUT_VARIABLE test_stdout
	ERROR_VARIABLE test_stderr
)
if (NOT test_result EQUAL 0)
	message(FATAL_ERROR
		"Running the installed-package consumer failed.\n"
		"${test_stdout}\n${test_stderr}"
	)
endif ()

message(STATUS "Installed Waveletpp package was consumed successfully.")
