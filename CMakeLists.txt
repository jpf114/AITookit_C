cmake_minimum_required(VERSION 3.21)

# vcpkg 全局模式：依赖预先安装到 $VCPKG_ROOT/installed，不使用 manifest
set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "Disable vcpkg manifest mode" FORCE)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
    else()
        message(FATAL_ERROR "VCPKG_ROOT environment variable is not set. Please set it to your vcpkg installation directory.")
    endif()
endif()

project(ai-toolkit-c VERSION 1.0.0 LANGUAGES CXX)

if(NOT DEFINED VCPKG_INSTALLED_DIR)
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_INSTALLED_DIR "$ENV{VCPKG_ROOT}/installed" CACHE PATH "vcpkg global install directory")
    else()
        message(FATAL_ERROR "VCPKG_ROOT is not set. Install dependencies globally and set VCPKG_ROOT before configuring.")
    endif()
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Default install path" FORCE)
endif()

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_STATIC_QT "Link Qt statically (requires Qt commercial license or LGPL compliance)" OFF)
option(AI_ENABLE_CUDA "Enable CUDA GPU inference (requires CUDA Toolkit and onnxruntime GPU build)" OFF)
if(USE_STATIC_QT)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
    message(WARNING "Static Qt linking enabled. Ensure LGPL v3 compliance or hold a Qt commercial license.")
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Svg Network REQUIRED)
find_package(Qt6 COMPONENTS LinguistTools QUIET)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs videoio)
find_package(onnxruntime CONFIG REQUIRED)

if(EXISTS "${CMAKE_SOURCE_DIR}/src/CMakeLists.txt")
    add_subdirectory(src)
else()
    message(STATUS "Skipping src: ${CMAKE_SOURCE_DIR}/src/CMakeLists.txt not found yet")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/copy_runtime_deps.cmake")
    include("${CMAKE_SOURCE_DIR}/cmake/copy_runtime_deps.cmake")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Skipping tests: ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt not found yet")
endif()

if(MSVC)
    if(DEFINED ENV{VCToolsRedistDir})
        set(AI_VC_REDIST_DIR "$ENV{VCToolsRedistDir}/x64/Microsoft.VC143.CRT")
    else()
        set(_vswhere "C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe")
        if(NOT EXISTS "${_vswhere}")
            set(_vswhere "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer/vswhere.exe")
        endif()
        if(EXISTS "${_vswhere}")
            execute_process(
                COMMAND "${_vswhere}" -latest -property installationPath
                OUTPUT_VARIABLE _vs_install_path
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(_vs_install_path)
                file(GLOB _vc_versions LIST_DIRECTORIES TRUE "${_vs_install_path}/VC/Redist/MSVC/*")
                if(_vc_versions)
                    set(_numeric_versions "")
                    foreach(_v IN LISTS _vc_versions)
                        get_filename_component(_vname "${_v}" NAME)
                        if(_vname MATCHES "^[0-9]")
                            list(APPEND _numeric_versions "${_v}")
                        endif()
                    endforeach()
                    if(_numeric_versions)
                        list(SORT _numeric_versions COMPARE NATURAL ORDER DESCENDING)
                        list(GET _numeric_versions 0 _latest_vc)
                        set(_candidate "${_latest_vc}/x64/Microsoft.VC143.CRT")
                        if(EXISTS "${_candidate}")
                            set(AI_VC_REDIST_DIR "${_candidate}")
                        endif()
                    endif()
                endif()
            endif()
        endif()
    endif()
    if(DEFINED AI_VC_REDIST_DIR AND EXISTS "${AI_VC_REDIST_DIR}")
        file(TO_CMAKE_PATH "${AI_VC_REDIST_DIR}" AI_VC_REDIST_DIR)
        message(STATUS "VC++ runtime found: ${AI_VC_REDIST_DIR}")
    else()
        set(AI_VC_REDIST_DIR "")
        message(WARNING "VC++ runtime not found, install will not include CRT DLLs")
    endif()
endif()

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/install_runtime_deps.cmake.in"
    "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake"
    @ONLY
)

if(EXISTS "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake")
    install(SCRIPT "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake")
endif()

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/scripts/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/scripts"
)

install(
    FILES
        "${CMAKE_SOURCE_DIR}/PRIVACY.md"
        "${CMAKE_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
        "${CMAKE_SOURCE_DIR}/docs/USER_GUIDE.md"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/doc"
)

install(
    FILES "${CMAKE_SOURCE_DIR}/resources/model_catalog.json"
    DESTINATION "${CMAKE_INSTALL_DATADIR}"
)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/models/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/models"
    FILES_MATCHING PATTERN "*.json" PATTERN "*.onnx"
)

set(CPACK_PACKAGE_NAME "AIToolkit")
set(CPACK_PACKAGE_VENDOR "AIToolkit")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "基于YOLO的AI目标检测桌面工具")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AIToolkit")
set(CPACK_PACKAGE_EXECUTABLES "ai_toolkit_c;AI检测工具")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "NSIS;ZIP")
set(CPACK_NSIS_DISPLAY_NAME "AI检测工具 ${PROJECT_VERSION}")
set(CPACK_NSIS_PACKAGE_NAME "AI检测工具")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\AI检测工具.lnk' '$INSTDIR\\\\bin\\\\ai_toolkit_c.exe'"
)
set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$SMPROGRAMS\\\\$START_MENU\\\\AI检测工具.lnk'"
)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)

if(EXISTS "${CMAKE_SOURCE_DIR}/resources/icons/app_icon.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\ai_toolkit_c.exe")
endif()

include(CPack)
