# Copyright (c) 2015-2016 Vittorio Romeo
# License: Academic Free License ("AFL") v. 3.0
# AFL License page: http://opensource.org/licenses/AFL-3.0
# http://vittorioromeo.info | vittorio.romeo@outlook.com

cmake_minimum_required(VERSION 3.6)

# Include `vrm_cmake`.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake/"
    "${CMAKE_SOURCE_DIR}/../vrm_cmake/cmake/"
    "${CMAKE_SOURCE_DIR}/extlibs/vrm_cmake/cmake/")

include(vrm_cmake)

# Basic project setup.
project(orizzonte CXX)
enable_testing()

vrm_cmake_init_project_name_upper()
vrm_cmake_init_project_source_dir()
vrm_cmake_init_project_common_module_paths()

vrm_cmake_find_extlib(vrm_pp)
vrm_cmake_find_extlib(vrm_core)
vrm_cmake_find_extlib(concurrentqueue)

# TODO: improve/remove MinGW devel environment setup
if(DEFINED WIN_MINGW_DEVEL)
#{
    add_definitions("-DORIZZONTE_USE_MEGANZ_MINGW_THREAD")
#}
endif()

# TODO:
# include_directories("/home/vittorioromeo/temp/mettle/include/")

# Alias the include directory.
set(ORIZZONTE_INC_DIR "${ORIZZONTE_SOURCE_DIR}/include/")
include_directories("${ORIZZONTE_INC_DIR}")

# Other compiler flags.
vrm_cmake_add_common_compiler_flags()

# TODO: improve/remove MinGW devel environment setup
if(NOT DEFINED WIN_MINGW_DEVEL)
#{
    set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
#}
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
vrm_cmake_add_common_compiler_flags_suggest_attribute()

# The `check` target runs all tests.
vrm_check_target()

# Setup subdirectories.
add_subdirectory(test)

# Create header-only install target (automatically glob)
vrm_cmake_header_only_install_glob("${ORIZZONTE_INC_DIR}" "include")


# add_executable(particles "${ORIZZONTE_SOURCE_DIR}/example/particles.cpp")
# add_executable(pres_code "${ORIZZONTE_SOURCE_DIR}/example/pres_code.cpp")

if(DEFINED WIN_MINGW_DEVEL)
#{
    # target_link_libraries(particles boost_system boost_thread)
    # target_link_libraries(pres_code boost_system boost_thread sfml-system sfml-graphics sfml-window)
#}
else()
#{
    # target_link_libraries(particles pthread)
    # target_link_libraries(pres_code pthread sfml-system sfml-graphics sfml-window)
#}
endif()
