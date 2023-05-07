cmake_minimum_required(VERSION 3.15)

project(oregano)

# --------------
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

include(cmake/CompilerWarnings.cmake)
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/Conan.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(NOMINMAX)

# --------------
if(BUILD_COVERAGE)
    if(NOT BUILD_TEST)
        message(FATAL_ERROR "To have coverage you must build with -DBUILD_TEST")
    endif()

    message(STATUS "Building coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E rm -rf coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory coverage
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
        COMMAND lcov --no-external --capture --initial --directory . --output-file coverage/lcov_base.info
        COMMAND ctest --output-on-failure --stop-on-failure --test-dir ${CMAKE_BINARY_DIR}
        COMMAND lcov --no-external --capture --directory . --output-file coverage/lcov.info
        COMMAND lcov --add-tracefile coverage/lcov_base.info --add-tracefile coverage/lcov.info --output-file coverage/lcov.info
        COMMAND lcov --remove coverage/lcov.info '/usr/*' '*/googletest/*' '${CMAKE_SOURCE_DIR}/test/*' --output-file coverage/lcov.info
        COMMAND genhtml --demangle-cpp --no-function-coverage coverage/lcov.info -o coverage
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

file(GLOB_RECURSE gcda_files ${CMAKE_BINARY_DIR}/*.gcda)

add_subdirectory(src)

if(BUILD_TEST)
    message(STATUS "Building tests")

    enable_testing()
    add_subdirectory(test)
endif()
