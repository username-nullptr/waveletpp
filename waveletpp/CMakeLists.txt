# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

include(project)

set(target_name waveletpp)
add_project(${target_name})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/waveletpp
	FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
