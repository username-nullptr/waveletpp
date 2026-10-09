# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

function(add_project target_name)
	file(GLOB_RECURSE project_headers CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
	)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${project_headers})
	add_library(${target_name} INTERFACE)

	target_compile_features(${target_name} INTERFACE cxx_std_17)
	target_include_directories(${target_name} INTERFACE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	)
	set_target_properties(${target_name} PROPERTIES
		EXPORT_NAME waveletpp
	)
	add_library(waveletpp::waveletpp ALIAS ${target_name})

	install(TARGETS ${target_name}
		EXPORT WaveletppTargets
	)
endfunction()
