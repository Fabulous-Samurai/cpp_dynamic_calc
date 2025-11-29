cmake_minimum_required(VERSION 3.11)
project(cpp_dynamic_calc VERSION 2.5.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")



configure_file(
    "${CMAKE_SOURCE_DIR}/version.h.in"
    "${CMAKE_BINARY_DIR}/generated/version.h"
)

add_subdirectory(libs/FTXUI)


find_package(Threads REQUIRED)


add_executable(cpp_dynamic_calc
        dynamic_calc.cpp
        dynamic_calc.h
        main.cpp
        dynamic_calc_types.h
        iParser.h
        algebraic_parser.h
        algebraic_parser.cpp
        linear_system_parser.cpp
        linear_system_parser.h
        string_helpers.h
)

target_include_directories(cpp_dynamic_calc PRIVATE "${CMAKE_BINARY_DIR}/generated")

target_link_libraries(cpp_dynamic_calc 
    PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
    Threads::Threads
)


add_executable(run_tests
        tests.cpp
        dynamic_calc.cpp 
        dynamic_calc.h
        dynamic_calc_types.h
        iParser.h
        algebraic_parser.h
        algebraic_parser.cpp
        linear_system_parser.cpp
        linear_system_parser.h
        string_helpers.h
)

target_link_libraries(run_tests 
    PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
    Threads::Threads
)


add_executable(ast_drills ast_drills.cpp)

