cmake_minimum_required(VERSION 3.21)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
endif()

set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "Use shared global vcpkg installation" FORCE)

project(ai-toolkit-c VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Default install path" FORCE)
endif()

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Svg REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
find_package(onnxruntime CONFIG REQUIRED)

if(EXISTS "${CMAKE_SOURCE_DIR}/src/CMakeLists.txt")
    add_subdirectory(src)
else()
    message(STATUS "Skipping src: ${CMAKE_SOURCE_DIR}/src/CMakeLists.txt not found yet")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Skipping tests: ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt not found yet")
endif()

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/install_runtime_deps.cmake.in"
    "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake"
    @ONLY
)

if(EXISTS "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake")
    install(SCRIPT "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake")
endif()
