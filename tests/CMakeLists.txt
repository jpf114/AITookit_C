set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Test REQUIRED)

function(ai_copy_qt_windows_platform_plugin target)
    if(NOT WIN32 OR NOT DEFINED ENV{VCPKG_ROOT})
        return()
    endif()

    set(qt_platform_plugin
        "$ENV{VCPKG_ROOT}/installed/x64-windows/$<$<CONFIG:Debug>:debug/>Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll")

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${qt_platform_plugin}"
            "$<TARGET_FILE_DIR:${target}>/platforms/"
        VERBATIM
    )
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
ai_add_qt_test_if_exists(test_export_service.cpp)
ai_add_qt_test_if_exists(test_main_window.cpp)
ai_add_qt_test_if_exists(test_models_page.cpp)
ai_add_qt_test_if_exists(test_results_page.cpp)
ai_add_qt_test_if_exists(test_settings_page.cpp)

if(TARGET test_yolo_postprocess)
    target_link_libraries(
        test_yolo_postprocess
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

if(TARGET test_main_window)
    target_sources(
        test_main_window
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/ui/main_window.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/nav_panel.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/dialogs/onnx_setup_dialog.cpp
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
            Qt6::Gui
            Qt6::Widgets
            Qt6::Svg
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
            Qt6::Gui
            Qt6::Widgets
    )
endif()

if(ai_toolkit_test_targets)
    add_custom_target(ai_toolkit_tests DEPENDS ${ai_toolkit_test_targets})
endif()
