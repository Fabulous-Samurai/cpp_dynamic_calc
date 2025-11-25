cmake_minimum_required(VERSION 3.11)
project(cpp_dynamic_calc)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/FTXUI)

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

target_link_libraries(cpp_dynamic_calc 
    PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
)

find_package(Threads REQUIRED)
target_link_libraries(cpp_dynamic_calc PRIVATE Threads::Threads)

if(WIN32)
    target_link_options(cpp_dynamic_calc PRIVATE "-static-libgcc" "-static-libstdc++" "-Wl,-Bstatic" "-lstdc++" "-lwinpthread" "-Wl,-Bdynamic")
else()
    target_link_options(cpp_dynamic_calc PRIVATE "-static")
endif()