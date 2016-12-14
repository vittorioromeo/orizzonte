# Add a custom target for the unit tests.
add_custom_target(tests COMMENT "Build all the unit tests.")

# `check` depends on `tests`.
add_dependencies(check tests)

# Include directories.
include_directories(${ORIZZONTE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE INSTALL_FILES_LIST "${CMAKE_CURRENT_LIST_DIR}/orizzonte/testbed/*.hpp")
file(GLOB_RECURSE INSTALL_FILES_LIST "${CMAKE_CURRENT_LIST_DIR}/orizzonte/*.hpp")
add_library(HEADER_ONLY_TARGET3 STATIC ${INSTALL_FILES_LIST})
set_source_files_properties(${INSTALL_FILES_LIST} PROPERTIES HEADER_FILE_ONLY 1)
set_target_properties(HEADER_ONLY_TARGET3 PROPERTIES LINKER_LANGUAGE CXX)

# Generate all the header unit tests.
# vrm_cmake_generate_public_header_tests_glob("*.hpp" "${ECST_SOURCE_DIR}/include")

# # Link `pthread` to every generated header test.
# foreach(_x IN LISTS vrm_cmake_out)
# #{
#     target_link_libraries(${_x} pthread)
# #}
# endforeach()

# Generate all the unit tests.
vrm_cmake_generate_unit_tests_glob("${ORIZZONTE_SOURCE_DIR}/test/*.cpp")

macro(augment_test x xname flags)
#{
    set(AUGMENTED_TEST_NAME "${x}_${xname}")
    get_target_property(xsrcs ${x} SOURCES)

    add_executable("${AUGMENTED_TEST_NAME}" EXCLUDE_FROM_ALL ${xsrcs})
    vrm_cmake_add_unit_test("${AUGMENTED_TEST_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${_x})
    target_compile_options("${AUGMENTED_TEST_NAME}" PUBLIC ${ARGN})
#}
endmacro()

# Link `pthread` to every generated test.
foreach(_x IN LISTS vrm_cmake_out)
#{
    target_compile_options(${_x} PUBLIC "-fdiagnostics-color=always")

    # TODO: to utility functions in vrm_cmake? to option?

    # TODO: improve/remove MinGW devel environment setup
    if(NOT DEFINED WIN_MINGW_DEVEL)
    #{
        get_target_property(xsrcs ${_x} SOURCES)

        augment_test(${_x} asan "-fdiagnostics-color=always" "-fsanitize=address")
        target_link_libraries("${AUGMENTED_TEST_NAME}" "-lasan")

        augment_test(${_x} tsan "-fdiagnostics-color=always" "-fsanitize=thread")
        target_link_libraries("${AUGMENTED_TEST_NAME}" "-ltsan")

        augment_test(${_x} ubsan "-fdiagnostics-color=always" "-fsanitize=undefined")
        target_link_libraries("${AUGMENTED_TEST_NAME}" "-lubsan")

        augment_test(${_x} asan_ofast "-Ofast" "-fdiagnostics-color=always" "-fsanitize=address")
        target_link_libraries("${AUGMENTED_TEST_NAME}" "-lasan")

        augment_test(${_x} tsan_ofast "-Ofast" "-fdiagnostics-color=always" "-fsanitize=thread")
        target_link_libraries("${AUGMENTED_TEST_NAME}" "-ltsan")

        augment_test(${_x} ubsan_ofast "-Ofast" "-fdiagnostics-color=always" "-fsanitize=undefined")
        target_link_libraries("${AUGMENTED_TEST_NAME}" "-lubsan")
    #}
    endif()

    if(DEFINED WIN_MINGW_DEVEL)
    #{
        # target_link_libraries(${_x} boost_system boost_thread)
    #}
    else()
    #{
        target_link_libraries(${_x} pthread)
    #}
    endif()
#}
endforeach()
