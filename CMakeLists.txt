cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME zera-vein-rest)

project(${PROJECT_NAME}
    LANGUAGES CXX
    VERSION 1.0.0
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(FeatureSummary)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

find_package(Qt5 COMPONENTS Core Network CONFIG REQUIRED)
find_package(qhttpengine REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(VeinFramework REQUIRED)
find_package(ZenuxCore REQUIRED)
find_package(VfLogger REQUIRED)
pkg_check_modules(Systemd REQUIRED libsystemd)

enable_testing()

write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
    )

configure_file(${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake @ONLY)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(systemd)
add_subdirectory(testlib)
add_subdirectory(tests)
