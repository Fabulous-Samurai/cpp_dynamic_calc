# Changelog

All notable changes to the C++ Dynamic Calculation Engine (Ogulator) will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.5.0] - 2024-11-29

### 🎉 Major Features Added

#### New Calculation Modes
- **Statistics Engine**: Complete statistical analysis suite
  - Descriptive statistics (mean, median, mode, standard deviation)
  - Distribution functions (normal, t-distribution, chi-squared)
  - Hypothesis testing (t-tests, chi-squared tests, ANOVA)
  - Correlation and linear regression analysis
  - Time series analysis (moving averages, exponential smoothing)

- **Unit Conversion Engine**: Comprehensive dimensional analysis
  - 20+ unit types across physical domains
  - Temperature conversions with proper offset handling
  - Length, mass, time, energy, power, and velocity conversions
  - Unit compatibility validation and automatic error checking

- **Plotting Engine**: ASCII-based function visualization
  - Mathematical function plotting with customizable ranges
  - Real-time function evaluation and coordinate mapping
  - Support for trigonometric, polynomial, and logarithmic functions
  - Configurable plot dimensions and scale factors

- **Symbolic Engine**: Computer algebra system foundation
  - Expression tree manipulation (expandable framework)
  - Variable substitution and symbolic differentiation placeholders
  - Extensible architecture for future CAS integration

#### Enhanced User Interface
- **Mode Switching**: Seamless transitions between calculation domains
  - `mode algebraic`, `mode linear`, `mode stats`, `mode units`, `mode plot`, `mode symbolic`
  - Context-aware help system with mode-specific commands
  - Persistent mode state across calculations

- **Improved TUI Experience**:
  - Enhanced command history with mode-aware navigation
  - Color-coded output for different result types
  - Real-time mode indicator in the interface
  - Comprehensive help system with examples

### 🚀 Performance Optimizations

#### Memory Management Enhancements
- **Expression Memoization Cache**: Intelligent result caching system
  - Context-aware cache key generation with serialization
  - Automatic cache invalidation on mode switches
  - Significant performance boost for repeated calculations

- **Arena Allocator Improvements**: Enhanced AST node allocation
  - 64KB block-based allocation for contiguous memory layout
  - Reduced memory fragmentation and improved cache locality
  - Automatic cleanup with RAII principles

#### Parsing Optimizations
- **SafeMath Integration**: Overflow-protected arithmetic operations
  - `std::from_chars` for faster numeric parsing
  - Integer overflow detection with proper error handling
  - Type-safe operations across all calculation domains

### 🛡️ Stability & Error Handling

#### Enhanced Error Management
- **Extended CalcErr Enum**: Comprehensive error categorization
  - `UNIT_CONVERSION_ERROR`, `STATISTICS_ERROR`, `PLOTTING_ERROR`
  - `SYMBOLIC_ERROR`, `INVALID_FUNCTION_CALL`
  - Context-preserving error messages with detailed descriptions

- **Engine-Specific Error Types**: Domain-aware error handling
  - `EngineResult<T>` pattern for consistent error propagation
  - Specialized error contexts for each calculation engine
  - Type-safe error composition across engine boundaries

#### Robustness Improvements
- **Input Validation**: Enhanced parsing with better error recovery
  - Regex-based unit parsing with comprehensive validation
  - Statistical data validation for array operations
  - Function domain checking for plotting operations

### 🔧 Technical Architecture

#### Code Organization
- **Modular Engine Design**: Separated concerns with dedicated engines
  - `StatisticsEngine`, `UnitManager`, `PlotEngine`, `SymbolicEngine`
  - Consistent `EngineResult<T>` return patterns
  - Clean separation of mathematical domains

- **Enhanced Build System**: Improved CMake configuration
  - Updated to version 2.5.0 with proper versioning
  - Comprehensive source file organization
  - Parallel build support with thread linking

#### Dependencies & Compatibility
- **C++20 Features**: Enhanced use of modern C++ capabilities
  - `std::from_chars` for fast numeric conversions
  - `std::optional` and `std::variant` for type safety
  - `constexpr` improvements for compile-time optimizations

### 📚 Documentation & Testing

#### Comprehensive Documentation
- **Updated README.md**: Complete feature documentation
  - Architecture evolution explanation
  - Comprehensive usage examples for all modes
  - Technical deep-dive sections
  - Build and development guidelines

- **Enhanced Code Comments**: Detailed inline documentation
  - Function-level documentation with parameter descriptions
  - Algorithm explanations for complex mathematical operations
  - Architecture pattern explanations

#### Testing Infrastructure
- **Extended Test Suite**: Comprehensive testing across engines
  - Unit tests for statistical functions
  - Unit conversion validation tests
  - Error handling verification tests
  - Performance benchmarking capabilities

### 🔄 API Changes

#### New Public Interfaces
- `CalcEngine::GetUnitManager()` - Access to unit conversion system
- `CalcEngine::GetStatisticsEngine()` - Statistical analysis interface
- `CalcEngine::GetPlotEngine()` - Function plotting capabilities
- `CalcEngine::GetSymbolicEngine()` - Symbolic math foundation

#### Enhanced Existing APIs
- `CalcEngine::SetMode()` - Now supports 6 calculation modes
- `EvaluateWithContext()` - Enhanced context handling with caching
- Error result types now include engine-specific error categories

### 🐛 Bug Fixes
- Fixed memory leaks in AST node allocation
- Resolved parsing issues with complex nested expressions
- Corrected operator precedence handling in symbolic expressions
- Fixed thread safety issues in concurrent evaluation scenarios

---

## [2.0.0] - Previous Release

### 🎯 Core Features
- **Algebraic Calculator**: Advanced expression parsing with AST evaluation
- **Linear System Solver**: Matrix operations and equation solving
- **FTXUI Integration**: Modern terminal user interface
- **Exception-Free Architecture**: `EvalResult<T>` error handling pattern
- **Thread-Safe Operations**: Concurrent expression evaluation
- **Command History**: Interactive navigation and calculation replay

### 🏗️ Architecture Foundation
- **Strategy Pattern**: Pluggable parsing engines
- **Arena Memory Management**: Efficient AST node allocation
- **Shunting-Yard Parser**: Proper operator precedence handling
- **Context-Aware Evaluation**: Variable support with `Ans` functionality

### 🧮 Mathematical Capabilities
- **Basic Operations**: `+`, `-`, `*`, `/`, `^`, `%`
- **Scientific Functions**: Trigonometric, logarithmic, exponential
- **Advanced Math**: `sqrt`, `abs`, `min`, `max`, inverse trig functions
- **Linear Algebra**: N-variable system solving with natural language input

### 🔧 Technical Features
- **C++20 Compliance**: Modern C++ patterns and best practices
- **CMake Build System**: Cross-platform compilation support
- **Zero-Overhead Error Handling**: `std::optional` and `std::variant`
- **Memory Efficiency**: Custom allocators and RAII patterns

---

## [1.0.0] - Initial Release

### 🌟 Foundation Release
- Basic arithmetic calculator functionality
- Simple command-line interface
- Core expression parsing capabilities
- Foundation architecture for future expansion

---

## Future Roadmap

### Planned Features (v3.0.0)
- **Computer Algebra System**: Full symbolic computation
- **3D Plotting**: Advanced visualization capabilities
- **Database Integration**: Persistent calculation history
- **Plugin System**: User-extensible calculation engines
- **Web Interface**: Browser-based calculation interface

### Performance Goals
- **JIT Compilation**: Runtime optimization for repeated expressions
- **SIMD Operations**: Vectorized mathematical operations
- **GPU Acceleration**: CUDA/OpenCL integration for intensive calculations
- **Distributed Computing**: Multi-node calculation support

### Developer Experience
- **IDE Integration**: Visual Studio Code extension
- **API Documentation**: Comprehensive developer reference
- **Benchmark Suite**: Performance regression testing
- **Continuous Integration**: Automated testing and deployment

---

*For more details about any release, see the corresponding Git tags and commit history.*