set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Test REQUIRED)

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
    add_test(NAME "${test_name}" COMMAND "${test_name}")
    set(ai_toolkit_test_targets ${ai_toolkit_test_targets} "${test_name}" PARENT_SCOPE)
endfunction()

ai_add_qt_test_if_exists(test_model_manifest.cpp)
ai_add_qt_test_if_exists(test_settings_store.cpp)
ai_add_qt_test_if_exists(test_yolo_postprocess.cpp)
ai_add_qt_test_if_exists(test_export_service.cpp)

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

if(ai_toolkit_test_targets)
    add_custom_target(ai_toolkit_tests DEPENDS ${ai_toolkit_test_targets})
endif()
