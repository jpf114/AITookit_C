set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

function(ai_collect_existing_sources out_var)
    set(existing_sources "")
    foreach(source IN LISTS ARGN)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
            list(APPEND existing_sources "${source}")
        endif()
    endforeach()
    set(${out_var} "${existing_sources}" PARENT_SCOPE)
endfunction()

function(ai_define_module target)
    set(options)
    set(one_value_args)
    set(multi_value_args SOURCES LINK_LIBS)
    cmake_parse_arguments(AI_MOD "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    ai_collect_existing_sources(module_sources ${AI_MOD_SOURCES})

    if(module_sources)
        add_library(${target} STATIC ${module_sources})
        target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
        if(AI_MOD_LINK_LIBS)
            target_link_libraries(${target} PUBLIC ${AI_MOD_LINK_LIBS})
        endif()
    else()
        add_library(${target} INTERFACE)
        target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endfunction()

function(ai_resolve_first_available_target out_var)
    foreach(candidate IN LISTS ARGN)
        if(TARGET ${candidate})
            set(${out_var} ${candidate} PARENT_SCOPE)
            return()
        endif()
    endforeach()

    set(${out_var} "" PARENT_SCOPE)
endfunction()

function(ai_copy_qt_windows_platform_plugin target)
    if(NOT WIN32)
        return()
    endif()

    if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins/platforms/qwindows.dll")
        set(qt_plugins_root "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows")
        set(qt_platform_plugin
            "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/$<$<CONFIG:Debug>:debug/>Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll")
    else()
        message(WARNING "Qt Windows platform plugin not found in vcpkg_installed. Qt plugins will not be copied.")
        return()
    endif()

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${qt_platform_plugin}"
            "$<TARGET_FILE_DIR:${target}>/platforms/"
        VERBATIM
    )

    set(qt_imageformats_dir "${qt_plugins_root}/Qt6/plugins/imageformats")
    if(EXISTS "${qt_imageformats_dir}")
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

ai_resolve_first_available_target(ai_opencv_core_target OpenCV::opencv_core opencv_core)
ai_resolve_first_available_target(ai_opencv_imgproc_target OpenCV::opencv_imgproc opencv_imgproc)
ai_resolve_first_available_target(ai_opencv_imgcodecs_target OpenCV::opencv_imgcodecs opencv_imgcodecs)
ai_resolve_first_available_target(ai_opencv_videoio_target OpenCV::opencv_videoio opencv_videoio)

if(NOT ai_opencv_core_target)
    message(FATAL_ERROR "Unable to resolve an OpenCV core target")
endif()

if(NOT ai_opencv_imgproc_target)
    message(FATAL_ERROR "Unable to resolve an OpenCV imgproc target")
endif()

if(NOT ai_opencv_imgcodecs_target)
    message(FATAL_ERROR "Unable to resolve an OpenCV imgcodecs target")
endif()

ai_define_module(
    ai_core
    SOURCES
        core/app_paths.cpp
        core/image_algorithms.cpp
        core/json_utils.cpp
        core/model_manifest.cpp
        core/settings_store.cpp
        core/unicode_path.cpp
    LINK_LIBS
        Qt6::Core
        Qt6::Gui
)

ai_define_module(
    ai_runtime
    SOURCES
        runtime/onnx_backend.cpp
        runtime/backend_registry.cpp
    LINK_LIBS
        ai_core
        onnxruntime::onnxruntime
)

ai_define_module(
    ai_models
    SOURCES
        models/yolo_detection_model.cpp
        models/postprocess_registry.cpp
        models/classification_model.cpp
        models/segmentation_model.cpp
        models/inference_backend.cpp
        runtime/onnx_plugin.cpp
    LINK_LIBS
        ai_core
        ai_runtime
        ${ai_opencv_core_target}
        ${ai_opencv_imgproc_target}
)

ai_define_module(
    ai_services
    SOURCES
        services/model_service.cpp
        services/inference_service.cpp
        services/inference_worker.cpp
        services/export_service.cpp
        services/unicode_io.cpp
    LINK_LIBS
        ai_core
        ai_models
        Qt6::Gui
        ${ai_opencv_imgcodecs_target}
        ${ai_opencv_videoio_target}
)

ai_collect_existing_sources(
    ai_toolkit_c_sources
    app/main.cpp
    app/app_bootstrap.cpp
    app/crash_handler.cpp
    ui/main_window.cpp
    ui/app_controller.cpp
    ui/image_utils.cpp
    ui/nav_panel.cpp
    ui/dialogs/onnx_setup_dialog.cpp
    ui/dialogs/model_catalog_dialog.cpp
    ui/pages/home_page.cpp
    ui/pages/models_page.cpp
    ui/pages/inference_page.cpp
    ui/pages/results_page.cpp
    ui/pages/settings_page.cpp
    ui/widgets/image_preview_widget.cpp
)

if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/resources/icons/app_icon.rc")
    list(APPEND ai_toolkit_c_sources "${CMAKE_SOURCE_DIR}/resources/icons/app_icon.rc")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/resources/icons/app_icon.qrc")
    list(APPEND ai_toolkit_c_sources "${CMAKE_SOURCE_DIR}/resources/icons/app_icon.qrc")
endif()

if(NOT ai_toolkit_c_sources)
    message(FATAL_ERROR "No sources found for ai_toolkit_c.")
endif()

add_executable(ai_toolkit_c ${ai_toolkit_c_sources})
target_include_directories(ai_toolkit_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
    ai_toolkit_c
    PRIVATE
        ai_services
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
)

if(WIN32)
    target_link_libraries(ai_toolkit_c PRIVATE DbgHelp)
endif()
ai_copy_qt_windows_platform_plugin(ai_toolkit_c)

install(TARGETS ai_toolkit_c RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/resources/translations/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/translations
    FILES_MATCHING PATTERN "*.qm"
)

if(EXISTS "${CMAKE_SOURCE_DIR}/resources/themes/app.qss")
    install(
        FILES "${CMAKE_SOURCE_DIR}/resources/themes/app.qss"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/themes
    )
endif()
