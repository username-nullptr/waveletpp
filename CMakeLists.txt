cmake_minimum_required(VERSION 3.12)
set(PRO_VERSION 0.1.0)
set(PRO_NAME waveletpp)

project(${PRO_NAME} VERSION ${PRO_VERSION} LANGUAGES C CXX)
add_definitions(-DLIBGS_VERSION_STR="${PRO_VERSION}-dev")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

include(utility)
check_compiler_version(CMAKE_CXX_STANDARD)

message(STATUS "")
message(STATUS "${PRO_NAME}: using C++: " ${CMAKE_CXX_STANDARD})
message(STATUS "${PRO_NAME}: build type: " ${CMAKE_BUILD_TYPE})
message(STATUS "${PRO_NAME}: install prefix: " ${CMAKE_INSTALL_PREFIX})

set(output_dir ${CMAKE_BINARY_DIR}/output)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
	add_definitions(-DLIBGS_DEBUG)
endif()

include_directories(.)
option(BUILD_EXAMPLES "-- ${PRO_NAME}: enable this to build the examples" ON)

if (BUILD_EXAMPLES)
	message(STATUS "${PRO_NAME}: enable this to build the examples.")
	add_subdirectory(examples)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES waveletpp.hpp)
install(DIRECTORY waveletpp/ DESTINATION include/waveletpp/ FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(FILES waveletpp.hpp DESTINATION include/)