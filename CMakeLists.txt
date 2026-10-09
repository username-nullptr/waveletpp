# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 2)
set(PATCH_VERSION 0)

set(PRO_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set(PRO_NAME Waveletpp)

project(${PRO_NAME} VERSION ${PRO_VERSION} LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(platform)
include(warnings)
include(options)

include(test)
include(CTest)

add_subdirectory(waveletpp)

# Generate the installed package after all exported targets have been defined.
include(provider)

if (BUILD_TESTING)
	add_subdirectory(test)
endif ()

if (WAVELETPP_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif ()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES waveletpp.hpp)
install(FILES waveletpp.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
