# SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
# SPDX-License-Identifier: MIT

set(WAVELETPP_INSTALL_CMAKEDIR
	"${CMAKE_INSTALL_LIBDIR}/cmake/${PRO_NAME}"
	CACHE STRING "Install path for ${PRO_NAME} CMake package files."
)
configure_package_config_file (
	${PROJECT_SOURCE_DIR}/cmake/WaveletppConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/WaveletppConfig.cmake
	INSTALL_DESTINATION ${WAVELETPP_INSTALL_CMAKEDIR}
)
write_basic_package_version_file (
	${CMAKE_CURRENT_BINARY_DIR}/WaveletppConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMinorVersion
)
install(EXPORT WaveletppTargets
	FILE WaveletppTargets.cmake
	NAMESPACE waveletpp::
	DESTINATION ${WAVELETPP_INSTALL_CMAKEDIR}
)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/WaveletppConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/WaveletppConfigVersion.cmake
	DESTINATION ${WAVELETPP_INSTALL_CMAKEDIR}
)
