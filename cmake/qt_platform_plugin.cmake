function(ai_copy_qt_windows_platform_plugin target)
    if(NOT WIN32)
        return()
    endif()

    set(qt_plugins_root "${VCPKG_INSTALLED_DIR}/x64-windows")
    if(NOT EXISTS "${qt_plugins_root}/Qt6/plugins/platforms/qwindows.dll")
        message(WARNING "Qt Windows platform plugin not found in vcpkg installed dir. Qt plugins will not be copied.")
        return()
    endif()

    set(qt_platform_plugins
        "${qt_plugins_root}/$<$<CONFIG:Debug>:debug/>Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
        "${qt_plugins_root}/$<$<CONFIG:Debug>:debug/>Qt6/plugins/platforms/qminimal$<$<CONFIG:Debug>:d>.dll"
        "${qt_plugins_root}/$<$<CONFIG:Debug>:debug/>Qt6/plugins/platforms/qoffscreen$<$<CONFIG:Debug>:d>.dll"
    )

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/platforms"
        VERBATIM
    )

    foreach(plugin IN LISTS qt_platform_plugins)
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${plugin}"
                "$<TARGET_FILE_DIR:${target}>/platforms/"
            VERBATIM
        )
    endforeach()

    set(qt_imageformats_dir "${qt_plugins_root}/$<$<CONFIG:Debug>:debug/>Qt6/plugins/imageformats")
    if(EXISTS "${qt_plugins_root}/Qt6/plugins/imageformats")
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/imageformats"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${qt_imageformats_dir}"
                "$<TARGET_FILE_DIR:${target}>/imageformats"
            VERBATIM
        )
    endif()
endfunction()
