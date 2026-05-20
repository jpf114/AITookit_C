if(NOT WIN32)
    return()
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/bin")
    set(vcpkg_bin_dir "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/bin")
    set(qt_plugins_dir "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins")
elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/installed/x64-windows/bin")
    set(vcpkg_bin_dir "$ENV{VCPKG_ROOT}/installed/x64-windows/bin")
    set(qt_plugins_dir "$ENV{VCPKG_ROOT}/installed/x64-windows/Qt6/plugins")
else()
    return()
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(config_dirs "${CMAKE_BINARY_DIR}/Debug" "${CMAKE_BINARY_DIR}/Release")
else()
    set(config_dirs "${CMAKE_BINARY_DIR}")
endif()

set(runtime_dlls
    onnxruntime.dll
    Qt6Core.dll
    Qt6Gui.dll
    Qt6Widgets.dll
    Qt6Svg.dll
    Qt6Test.dll
    opencv_core4.dll
    opencv_imgproc4.dll
    opencv_imgcodecs4.dll
    opencv_videoio4.dll
)

foreach(dll_name IN LISTS runtime_dlls)
    if(EXISTS "${vcpkg_bin_dir}/${dll_name}")
        foreach(dest_dir IN LISTS config_dirs)
            file(INSTALL
                DESTINATION "${dest_dir}"
                TYPE SHARED_LIBRARY
                FILES "${vcpkg_bin_dir}/${dll_name}"
            )
        endforeach()
    endif()
endforeach()

foreach(dest_dir IN LISTS config_dirs)
    if(EXISTS "${qt_plugins_dir}/platforms/qwindows.dll")
        file(INSTALL
            DESTINATION "${dest_dir}/platforms"
            TYPE SHARED_LIBRARY
            FILES "${qt_plugins_dir}/platforms/qwindows.dll"
        )
    endif()

    set(image_format_plugins
        qjpeg.dll
        qsvg.dll
        qgif.dll
        qico.dll
        qtiff.dll
        qwebp.dll
    )

    foreach(plugin IN LISTS image_format_plugins)
        if(EXISTS "${qt_plugins_dir}/imageformats/${plugin}")
            file(INSTALL
                DESTINATION "${dest_dir}/imageformats"
                TYPE SHARED_LIBRARY
                FILES "${qt_plugins_dir}/imageformats/${plugin}"
            )
        endif()
    endforeach()
endforeach()
