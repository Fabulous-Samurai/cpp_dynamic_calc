cmake_minimum_required(VERSION 3.11)

# Proje ve Versiyon
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


include_directories(
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/generated"
)


add_executable(cpp_dynamic_calc
        src/main.cpp
        src/dynamic_calc.cpp
        src/algebraic_parser.cpp
        src/linear_system_parser.cpp
        src/string_helpers.cpp
        src/unit_manager.cpp
        src/unit_parser.cpp
        src/statistics_engine.cpp
        src/symbolic_engine.cpp
        src/plot_engine.cpp
        
        include/dynamic_calc.h
        include/dynamic_calc_types.h
        include/iParser.h
        include/algebraic_parser.h
        include/linear_system_parser.h
        include/string_helpers.h
        include/unit_manager.h
        include/unit_parser.h
        include/statistics_engine.h
        include/symbolic_engine.h
        include/plot_engine.h
)

target_link_libraries(cpp_dynamic_calc 
    PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
    Threads::Threads
)


add_executable(run_tests
        tests/tests.cpp
        src/dynamic_calc.cpp
        src/algebraic_parser.cpp
        src/linear_system_parser.cpp
        src/string_helpers.cpp
        src/unit_manager.cpp
        src/unit_parser.cpp
        src/statistics_engine.cpp
        src/symbolic_engine.cpp
        src/plot_engine.cpp
        
        include/dynamic_calc.h
        include/string_helpers.h
        include/unit_manager.h
        include/unit_parser.h
        include/statistics_engine.h
        include/symbolic_engine.h
        include/plot_engine.h
)

target_link_libraries(run_tests 
    PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
    Threads::Threads
)


add_executable(ast_drills tests/ast_drills.cpp)
target_include_directories(ast_drills PRIVATE "${CMAKE_SOURCE_DIR}/include")