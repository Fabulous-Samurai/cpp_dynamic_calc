cmake_minimum_required(VERSION 3.12)

# Project and Version
project(cpp_dynamic_calc VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable position independent code for nanobind
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable parallel compilation optimizations
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # Visual Studio parallel builds
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
elseif(CMAKE_GENERATOR MATCHES "Ninja" OR CMAKE_GENERATOR MATCHES "Unix Makefiles")
    # Ninja and Make parallel builds
    include(ProcessorCount)
    ProcessorCount(CPU_COUNT)
    if(NOT CPU_COUNT EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${CPU_COUNT})
        message(STATUS "Parallel build enabled with ${CPU_COUNT} cores")
    endif()
endif()

# Optimization flags for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG /fp:fast")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG -ffast-math")
    endif()
endif()

# Optional: Enable advanced features
option(ENABLE_ADVANCED_FEATURES "Enable advanced computational features" ON)
option(ENABLE_PARALLEL_BUILD "Enable parallel compilation" ON)

# Prefer Ninja generator for faster builds
set_property(GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG TRUE)

# Find Python for nanobind integration (REQUIRED for nanobind)
find_package(Python COMPONENTS Interpreter Development REQUIRED)
if(Python_FOUND)
    message(STATUS "Python found: ${Python_VERSION}")
    set(ENABLE_PYTHON_FFI ON)
    
    # Try to find nanobind
    find_package(nanobind QUIET)
    if(nanobind_FOUND)
        set(ENABLE_NANOBIND ON)
        add_definitions(-DENABLE_NANOBIND)
        message(STATUS "Nanobind found and enabled")
    else()
        # If not found, try to use FetchContent to download it
        include(FetchContent)
        message(STATUS "Nanobind not found - attempting to download via FetchContent...")
        FetchContent_Declare(
            nanobind
            GIT_REPOSITORY https://github.com/wjakob/nanobind.git
            GIT_TAG v2.9.2
        )
        FetchContent_MakeAvailable(nanobind)
        set(ENABLE_NANOBIND ON)
        add_definitions(-DENABLE_NANOBIND)
        message(STATUS "Nanobind downloaded and enabled via FetchContent")
    endif()
    
    add_definitions(-DENABLE_PYTHON_FFI)
else()
    message(STATUS "Python development libraries not found - building with limited Python support")
    set(ENABLE_PYTHON_FFI ON)  # Still enable for basic FFI
    set(ENABLE_NANOBIND OFF)
endif()

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

# Python source files (conditionally compiled)
set(PYTHON_SOURCES "")
set(PYTHON_HEADERS "")
if(ENABLE_PYTHON_FFI)
    set(PYTHON_SOURCES
        src/python_engine.cpp
        src/python_parser.cpp
        src/python_repl.cpp
    )
    set(PYTHON_HEADERS
        include/python_engine.h
        include/python_parser.h
        include/python_repl.h
    )
    include_directories(${Python_INCLUDE_DIRS})
endif()


# Enhanced source files with Eigen and parallel computing support
set(ENHANCED_SOURCES
    src/eigen_engine.cpp
    src/selective_dispatcher.cpp
    src/cpu_optimization.cpp
)

# Temporarily disable nanobind until headers are resolved
# Only include nanobind interface if nanobind is enabled
#if(ENABLE_NANOBIND)
#    list(APPEND ENHANCED_SOURCES src/nanobind_interface.cpp)
#endif()

set(ENHANCED_HEADERS
    include/eigen_engine.h
    include/selective_dispatcher.h
    include/cpu_optimization.h
)

# Temporarily disable nanobind until headers are resolved
# Only include nanobind interface header if nanobind is enabled
#if(ENABLE_NANOBIND)
#    list(APPEND ENHANCED_HEADERS include/nanobind_interface.h)
#endif()

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
        ${ENHANCED_SOURCES}
        ${PYTHON_SOURCES}
        
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
        ${ENHANCED_HEADERS}
        ${PYTHON_HEADERS}
)

target_link_libraries(cpp_dynamic_calc 
    PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
    Threads::Threads
)

# Link OpenMP if available
if(OpenMP_CXX_FOUND)
    target_link_libraries(cpp_dynamic_calc PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(cpp_dynamic_calc PRIVATE ENABLE_OPENMP)
    message(STATUS "Linking with OpenMP for parallel computing")
endif()

# Link Eigen if available
if(TARGET Eigen3::Eigen)
    target_link_libraries(cpp_dynamic_calc PRIVATE Eigen3::Eigen)
    message(STATUS "Linking with Eigen3")
endif()

# Link Python libraries if available
if(ENABLE_PYTHON_FFI AND Python_FOUND)
    target_link_libraries(cpp_dynamic_calc PRIVATE ${Python_LIBRARIES})
    target_include_directories(cpp_dynamic_calc PRIVATE ${Python_INCLUDE_DIRS})
    message(STATUS "Linking with Python libraries")
endif()

# Link nanobind if available
if(ENABLE_NANOBIND AND TARGET nanobind::nanobind)
    target_link_libraries(cpp_dynamic_calc PRIVATE nanobind::nanobind)
    message(STATUS "Linking with nanobind")
endif()

# Add Python and nanobind include paths for all targets
if(ENABLE_PYTHON_FFI)
    target_include_directories(cpp_dynamic_calc PRIVATE ${Python_INCLUDE_DIRS})
endif()

if(ENABLE_NANOBIND)
    # nanobind includes should be available after FetchContent
    target_include_directories(cpp_dynamic_calc PRIVATE 
        ${CMAKE_BINARY_DIR}/_deps/nanobind-src/include)
endif()

# Enable parallel computing flags
if(ENABLE_PARALLEL_BUILD)
    target_compile_definitions(cpp_dynamic_calc PRIVATE ENABLE_PARALLEL_COMPUTING)
    message(STATUS "Parallel computing support enabled")
endif()

# Enable parallel computing flags
if(ENABLE_PARALLEL_BUILD)
    target_compile_definitions(cpp_dynamic_calc PRIVATE ENABLE_PARALLEL_COMPUTING)
    message(STATUS "Parallel computing support enabled")
endif()


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
        ${PYTHON_SOURCES}
        
        include/dynamic_calc.h
        include/string_helpers.h
        include/unit_manager.h
        include/unit_parser.h
        include/statistics_engine.h
        include/symbolic_engine.h
        include/plot_engine.h
        ${PYTHON_HEADERS}
)

target_link_libraries(run_tests 
    PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
    Threads::Threads
)

# Link Python libraries for tests if available
if(ENABLE_PYTHON_FFI)
    target_link_libraries(run_tests PRIVATE Python::Python)
    target_include_directories(run_tests PRIVATE ${Python_INCLUDE_DIRS})
endif()


add_executable(ast_drills tests/ast_drills.cpp)
target_include_directories(ast_drills PRIVATE "${CMAKE_SOURCE_DIR}/include")