cmake_minimum_required(VERSION 3.10)
project(cpp_dynamic_calc)

set(CMAKE_CXX_STANDARD 20)

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
)
