set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Test REQUIRED)

function(ai_copy_qt_windows_platform_plugin target)
    if(NOT WIN32)
        return()
    endif()

    set(qt_plugins_root "${VCPKG_INSTALLED_DIR}/x64-windows")
    if(NOT EXISTS "${qt_plugins_root}/Qt6/plugins/platforms/qwindows.dll")
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

function(ai_add_qt_test_if_exists source_file)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
        return()
    endif()

    get_filename_component(test_name "${source_file}" NAME_WE)
    add_executable("${test_name}" "${source_file}")
    target_include_directories("${test_name}" PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(
        "${test_name}"
        PRIVATE
            ai_core
            Qt6::Core
            Qt6::Test
    )
    ai_copy_qt_windows_platform_plugin("${test_name}")
    add_test(NAME "${test_name}" COMMAND "${test_name}")
    set(ai_toolkit_test_targets ${ai_toolkit_test_targets} "${test_name}" PARENT_SCOPE)
endfunction()

ai_add_qt_test_if_exists(test_model_manifest.cpp)
ai_add_qt_test_if_exists(test_settings_store.cpp)
ai_add_qt_test_if_exists(test_yolo_postprocess.cpp)
ai_add_qt_test_if_exists(test_postprocess_registry.cpp)
ai_add_qt_test_if_exists(test_export_service.cpp)
ai_add_qt_test_if_exists(test_model_service.cpp)
ai_add_qt_test_if_exists(test_app_controller.cpp)
ai_add_qt_test_if_exists(test_main_window.cpp)
ai_add_qt_test_if_exists(test_models_page.cpp)
ai_add_qt_test_if_exists(test_results_page.cpp)
ai_add_qt_test_if_exists(test_settings_page.cpp)
ai_add_qt_test_if_exists(test_gui_validation.cpp)
ai_add_qt_test_if_exists(test_classification_model.cpp)
ai_add_qt_test_if_exists(test_segmentation_model.cpp)
ai_add_qt_test_if_exists(test_inference_service.cpp)
ai_add_qt_test_if_exists(test_backend_registry.cpp)
ai_add_qt_test_if_exists(test_unicode_io.cpp)
ai_add_qt_test_if_exists(test_inference_worker.cpp)

if(TARGET test_yolo_postprocess)
    target_link_libraries(
        test_yolo_postprocess
        PRIVATE
            ai_models
    )
endif()

if(TARGET test_postprocess_registry)
    target_link_libraries(
        test_postprocess_registry
        PRIVATE
            ai_models
    )
endif()

if(TARGET test_export_service)
    target_link_libraries(
        test_export_service
        PRIVATE
            ai_services
    )
endif()

if(TARGET test_model_service)
    target_link_libraries(
        test_model_service
        PRIVATE
            ai_services
    )
endif()

if(TARGET test_classification_model)
    target_link_libraries(
        test_classification_model
        PRIVATE
            ai_models
    )
endif()

if(TARGET test_segmentation_model)
    target_link_libraries(
        test_segmentation_model
        PRIVATE
            ai_models
    )
endif()

if(TARGET test_inference_service)
    target_link_libraries(
        test_inference_service
        PRIVATE
            ai_services
    )
endif()

if(TARGET test_backend_registry)
    target_link_libraries(
        test_backend_registry
        PRIVATE
            ai_runtime
            ai_models
    )
endif()

if(TARGET test_unicode_io)
    target_link_libraries(
        test_unicode_io
        PRIVATE
            ai_services
    )
endif()

if(TARGET test_inference_worker)
    target_link_libraries(
        test_inference_worker
        PRIVATE
            ai_services
    )
endif()

if(TARGET test_app_controller)
    target_sources(
        test_app_controller
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/ui/app_controller.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/image_utils.cpp
    )
    target_link_libraries(
        test_app_controller
        PRIVATE
            ai_services
            Qt6::Gui
    )
endif()

if(TARGET test_main_window)
    target_sources(
        test_main_window
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/ui/license_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/main_window.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/app_controller.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/image_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/nav_panel.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/dialogs/onnx_setup_dialog.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/dialogs/model_catalog_dialog.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/home_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/models_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/inference_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/results_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/settings_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/widgets/image_preview_widget.cpp
    )
    target_link_libraries(
        test_main_window
        PRIVATE
            ai_services
            ai_core
            Qt6::Gui
            Qt6::Widgets
            Qt6::Svg
            Qt6::Network
    )
endif()

if(TARGET test_models_page)
    target_sources(
        test_models_page
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/ui/pages/models_page.cpp
    )
    target_link_libraries(
        test_models_page
        PRIVATE
            Qt6::Gui
            Qt6::Widgets
    )
endif()

if(TARGET test_results_page)
    target_sources(
        test_results_page
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/ui/pages/results_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/image_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/widgets/image_preview_widget.cpp
    )
    target_link_libraries(
        test_results_page
        PRIVATE
            ai_services
            Qt6::Gui
            Qt6::Widgets
            Qt6::Svg
    )
endif()

if(TARGET test_settings_page)
    target_sources(
        test_settings_page
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/ui/pages/settings_page.cpp
    )
    target_link_libraries(
        test_settings_page
        PRIVATE
            ai_core
            Qt6::Gui
            Qt6::Widgets
            Qt6::Network
    )
endif()

if(ai_toolkit_test_targets)
    add_custom_target(ai_toolkit_tests DEPENDS ${ai_toolkit_test_targets})
endif()

if(TARGET test_gui_validation)
    target_sources(
        test_gui_validation
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/ui/license_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/main_window.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/app_controller.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/image_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/nav_panel.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/dialogs/onnx_setup_dialog.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/dialogs/model_catalog_dialog.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/home_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/models_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/inference_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/results_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/pages/settings_page.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/widgets/image_preview_widget.cpp
    )
    target_link_libraries(
        test_gui_validation
        PRIVATE
            ai_services
            ai_core
            Qt6::Gui
            Qt6::Widgets
            Qt6::Svg
            Qt6::Network
    )
endif()
