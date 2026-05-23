if(NOT WIN32)
    return()
endif()

set(vcpkg_root "${VCPKG_INSTALLED_DIR}/x64-windows")
if(NOT EXISTS "${vcpkg_root}/bin")
    return()
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(config_dirs "${CMAKE_BINARY_DIR}/Debug" "${CMAKE_BINARY_DIR}/Release")
else()
    set(config_dirs "${CMAKE_BINARY_DIR}")
endif()

set(runtime_dlls
    Qt6Core.dll
    Qt6Gui.dll
    Qt6Widgets.dll
    Qt6Svg.dll
    Qt6Network.dll
    double-conversion.dll
    pcre2-16.dll
    md4c.dll
    z.dll
    onnxruntime.dll
    onnxruntime_providers_shared.dll
    libprotobuf.dll
    libprotobuf-lite.dll
    abseil_dll.dll
    re2.dll
    opencv_core4.dll
    opencv_imgproc4.dll
    opencv_imgcodecs4.dll
    opencv_videoio4.dll
    libpng16.dll
    jpeg62.dll
    turbojpeg.dll
)

foreach(dll_name IN LISTS runtime_dlls)
    if(EXISTS "${vcpkg_root}/bin/${dll_name}")
        foreach(dest_dir IN LISTS config_dirs)
            file(INSTALL
                DESTINATION "${dest_dir}"
                TYPE SHARED_LIBRARY
                FILES "${vcpkg_root}/bin/${dll_name}"
            )
        endforeach()
    endif()
endforeach()

set(qt_plugins_dir "${vcpkg_root}/Qt6/plugins")

set(qt_plugin_map
    "platforms=qwindows.dll;qdirect2d.dll;qminimal.dll;qoffscreen.dll"
    "imageformats=qjpeg.dll;qsvg.dll;qgif.dll;qico.dll"
    "styles=qmodernwindowsstyle.dll"
    "iconengines=qsvgicon.dll"
)

foreach(entry IN LISTS qt_plugin_map)
    string(REGEX MATCH "^([^=]+)=(.*)$" _ "${entry}")
    set(plugin_type "${CMAKE_MATCH_1}")
    set(plugin_files "${CMAKE_MATCH_2}")
    string(REPLACE ";" ";" plugin_list "${plugin_files}")

    foreach(plugin_name IN LISTS plugin_list)
        set(plugin_path "${qt_plugins_dir}/${plugin_type}/${plugin_name}")
        if(EXISTS "${plugin_path}")
            foreach(dest_dir IN LISTS config_dirs)
                file(INSTALL
                    DESTINATION "${dest_dir}/${plugin_type}"
                    TYPE SHARED_LIBRARY
                    FILES "${plugin_path}"
                )
            endforeach()
        endif()
    endforeach()
endforeach()
