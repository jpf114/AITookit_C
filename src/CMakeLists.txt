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
        core/app_logger.cpp
        core/update_checker.cpp
    LINK_LIBS
        Qt6::Core
        Qt6::Gui
        Qt6::Network
)

if(TARGET ai_core)
    target_include_directories(ai_core PUBLIC "${CMAKE_BINARY_DIR}/generated")
endif()

ai_define_module(
    ai_runtime
    SOURCES
        runtime/onnx_backend.cpp
        runtime/backend_registry.cpp
        runtime/tensorrt_plugin.cpp
        runtime/openvino_plugin.cpp
    LINK_LIBS
        ai_core
        onnxruntime::onnxruntime
)

if(AI_ENABLE_CUDA)
    if(TARGET ai_runtime)
        target_compile_definitions(ai_runtime PUBLIC USE_CUDA)
        message(STATUS "CUDA GPU inference enabled (USE_CUDA)")
    endif()
endif()

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
    ui/license_utils.cpp
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
target_include_directories(ai_toolkit_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(
    ai_toolkit_c
    PRIVATE
        ai_services
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        Qt6::Concurrent
)

if(WIN32)
    target_link_libraries(ai_toolkit_c PRIVATE DbgHelp)
endif()
ai_copy_qt_windows_platform_plugin(ai_toolkit_c)

set(AI_TRANSLATION_TS_FILES
    "${CMAKE_SOURCE_DIR}/resources/translations/ai_toolkit_en.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/ai_toolkit_zh_CN.ts"
)
if(EXISTS "${CMAKE_SOURCE_DIR}/resources/translations/ai_toolkit_en.ts")
    if(TARGET Qt6::lrelease OR Qt6LinguistTools_FOUND)
        qt_add_translations(
            ai_toolkit_c
            TS_FILES ${AI_TRANSLATION_TS_FILES}
            QM_FILES_OUTPUT_VARIABLE ai_qm_files
        )
    else()
        find_program(AI_LRELEASE_EXECUTABLE lrelease HINTS "${VCPKG_INSTALLED_DIR}/x64-windows/tools/qt6/bin")
        if(AI_LRELEASE_EXECUTABLE)
            set(ai_qm_files "")
            foreach(ts_file IN LISTS AI_TRANSLATION_TS_FILES)
                get_filename_component(ts_name "${ts_file}" NAME_WE)
                set(qm_out "${CMAKE_CURRENT_BINARY_DIR}/${ts_name}.qm")
                add_custom_command(
                    OUTPUT "${qm_out}"
                    COMMAND "${AI_LRELEASE_EXECUTABLE}" "${ts_file}" -qm "${qm_out}"
                    DEPENDS "${ts_file}"
                    COMMENT "Generating ${ts_name}.qm"
                    VERBATIM
                )
                list(APPEND ai_qm_files "${qm_out}")
            endforeach()
            add_custom_target(ai_toolkit_translations DEPENDS ${ai_qm_files})
            add_dependencies(ai_toolkit_c ai_toolkit_translations)
        else()
            message(WARNING "Qt Linguist tools not found; translations will not be built.")
            set(ai_qm_files "")
        endif()
    endif()

    if(ai_qm_files)
    add_custom_command(
        TARGET ai_toolkit_c
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:ai_toolkit_c>/translations"
        COMMENT "Creating translations output directory"
        VERBATIM
    )
    foreach(qm_file IN LISTS ai_qm_files)
        add_custom_command(
            TARGET ai_toolkit_c
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${qm_file}"
                "$<TARGET_FILE_DIR:ai_toolkit_c>/translations/"
            COMMENT "Copying ${qm_file} to runtime translations directory"
            VERBATIM
        )
        endforeach()
    endif()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/resources/model_catalog.json")
    add_custom_command(
        TARGET ai_toolkit_c
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:ai_toolkit_c>/../share"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/resources/model_catalog.json"
            "$<TARGET_FILE_DIR:ai_toolkit_c>/../share/model_catalog.json"
        COMMENT "Copying model_catalog.json to runtime share directory"
        VERBATIM
    )
endif()

install(TARGETS ai_toolkit_c RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/resources/translations/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/translations
    FILES_MATCHING PATTERN "*.qm"
)

# qt_add_translations also installs .qm via the target; keep directory install as fallback.

if(EXISTS "${CMAKE_SOURCE_DIR}/resources/themes/app.qss")
    install(
        FILES "${CMAKE_SOURCE_DIR}/resources/themes/app.qss"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/themes
    )
endif()
