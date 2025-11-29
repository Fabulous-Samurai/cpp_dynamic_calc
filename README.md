# C++ Dynamic Calculation Engine (Ogulator)

> **A comprehensive mathematical computing platform combining high-performance parsing, scientific computation, and intuitive TUI interaction.**

![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Version](https://img.shields.io/badge/Version-2.5.0-brightgreen.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Build](https://img.shields.io/badge/Build-CMake%20%2B%20Ninja-orange.svg)

Ogulator is not just a calculator—it's a **multi-paradigm mathematical computation engine** that demonstrates advanced C++20 architecture patterns. From procedural arithmetic to sophisticated scientific analysis, this project showcases the evolution of modern C++ design principles including **Strategy Pattern**, **Arena Memory Management**, **Exception-Free Error Handling**, and **Type-Safe Concurrency**.

---

## 🎯 Key Features

### 🧮 **Multi-Mode Computation Engine**
- **Algebraic Mode:** Advanced expression parsing with AST-based evaluation
- **Linear System Mode:** Matrix operations and equation solving
- **Statistics Mode:** Comprehensive statistical analysis and hypothesis testing
- **Unit Conversion Mode:** Dimensional analysis with 20+ unit types
- **Plotting Mode:** ASCII-based function visualization
- **Symbolic Mode:** Computer algebra foundations (expandable)

### ⚡ **Performance & Architecture**
- **Arena Memory Management:** Custom 64KB block allocator for AST nodes
- **Expression Memoization:** Intelligent caching with context-aware invalidation
- **SafeMath Operations:** Overflow-protected arithmetic with `std::from_chars`
- **Exception-Free Design:** `EvalResult<T>` pattern with `std::optional` and error enums
- **Thread-Safe Evaluation:** Concurrent expression processing

### 🎨 **Modern TUI Interface (FTXUI)**
- **Interactive Terminal UI:** Real-time input with syntax highlighting
- **Command History:** Navigate previous calculations with arrow keys
- **Mode Switching:** Seamless transitions between calculation domains
- **Scrollable Output:** Full-featured log with color-coded results

---

## 🏗️ Architectural Evolution

### Phase I: Foundation (Exception-Free Error Handling)
**Problem:** Traditional exception-based error handling created performance overhead and unpredictable control flow.

**Solution:** Implemented `EvalResult<T>` wrapper using `std::optional` and `CalcErr` enum for deterministic, zero-overhead error propagation.

### Phase II: Performance Optimization (Arena + Memoization)
**Problem:** Frequent AST node allocation caused memory fragmentation and cache misses.

**Solution:** Custom Arena allocator with 64KB blocks and expression memoization cache for repeated evaluations.

### Phase III: Domain Expansion (Strategy Pattern)
**Problem:** Monolithic parser couldn't handle diverse mathematical domains.

**Solution:** Decoupled parsing strategies (`AlgebraicParser`, `LinearSystemParser`, etc.) with unified `IParser` interface.

### Phase IV: Scientific Computing (Specialized Engines)
**Problem:** Advanced mathematical operations required domain-specific expertise.

**Solution:** Dedicated engines for statistics, units, plotting, and symbolic computation with consistent `EngineResult` patterns.

---

## 🔧 Computation Capabilities

### 🧮 **Algebraic Engine**
```cpp
// Basic Operations
3 + 5 * 2^3 - sqrt(16)

// Advanced Functions
sin(45) + cos(30) + tan(60)
log(100) + ln(e) + exp(2)
abs(-5) + max(3,7) + min(2,9)

// With Variables (Ans)
5 * 3          // → 15
Ans + 10       // → 25 (uses previous result)
```

### 📊 **Statistics Engine**
```cpp
// Descriptive Statistics
data = [1,2,3,4,5,6,7,8,9,10]
mean(data)     // → 5.5
median(data)   // → 5.5
std_dev(data)  // → 3.03

// Hypothesis Testing
t_test(sample1, sample2)      // Two-sample t-test
chi_squared_test(obs, exp)    // Chi-squared test
correlation(x_data, y_data)   // Pearson correlation
```

### 🔄 **Unit Conversion Engine**
```cpp
// Length Conversions
convert(100, "cm", "m")     // → 1.0
convert(5, "ft", "in")      // → 60.0

// Temperature Conversions
convert(100, "C", "F")      // → 212.0
convert(273.15, "K", "C")   // → 0.0

// Complex Units
convert(60, "mph", "m/s")    // → 26.82
```

### 📐 **Linear System Solver**
```cpp
// Natural Language Input
"2x + 3y = 10; x - y = 1"
// → Solution: x = 2.6, y = 1.6

// Matrix Operations
"x + 2y + z = 6; 2x - y + 3z = 14; 3x + y - z = -2"
// → Solution: x = 1, y = 2, z = 1
```

### 📈 **Plotting Engine**
```cpp
// Function Plotting
plot("sin(x)", -10, 10)     // ASCII sine wave
plot("x^2", -5, 5)          // Parabola visualization
plot("log(x)", 0.1, 10)     // Logarithmic curve
```

---

## 🚀 Getting Started

### Prerequisites
- **C++20** compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- **CMake 3.11+**
- **Ninja** (recommended) or Make
- **Git** for submodule management

### Build Instructions

#### Windows (MSYS2/MinGW)
```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/cpp_dynamic_calc.git
cd cpp_dynamic_calc

# Configure and build
cmake -S . -B build -G "Ninja"
cmake --build build --parallel

# Run
.\build\cpp_dynamic_calc.exe
```

#### Linux/macOS
```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/cpp_dynamic_calc.git
cd cpp_dynamic_calc

# Configure and build
cmake -S . -B build
cmake --build build -j$(nproc)

# Run
./build/cpp_dynamic_calc
```

### Development Build
```bash
# Debug build with testing
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug

# Run tests
./cmake-build-debug/run_tests
```

---

## 🎮 Usage Guide

### Interactive Commands
```bash
# Mode Switching
mode algebraic    # Switch to algebraic calculator
mode linear      # Switch to linear system solver
mode stats       # Switch to statistics mode
mode units       # Switch to unit conversion
mode plot        # Switch to plotting mode

# Utility Commands
help            # Show command reference
clear           # Clear screen
exit            # Close application
history         # Show calculation history
```

### Example Session
```
Ogulator v2.5.0 - Multi-Modal Calculation Engine
> mode algebraic
Switched to Algebraic mode

> 3 + 5 * 2^3
Result: 43

> sin(90) + cos(0)
Result: 2

> mode stats
Switched to Statistics mode

> mean([1,2,3,4,5])
Result: 3

> mode units
Switched to Units mode

> convert(100, "cm", "m")
Result: 1 m
```

---

## 🧪 Testing

### Unit Tests
```bash
# Run all tests
./build/run_tests

# AST-specific tests
./build/ast_drills
```

### Manual Testing
```bash
# Performance benchmarking
time echo "sin(45) * cos(30) + tan(60)" | ./build/cpp_dynamic_calc

# Memory usage analysis
valgrind --tool=memcheck ./build/cpp_dynamic_calc
```

---

## 🔬 Technical Deep Dive

### Memory Management
- **Arena Allocator:** 64KB block-based allocation for AST nodes
- **RAII Patterns:** Automatic cleanup with smart pointers
- **Cache-Friendly:** Contiguous memory layout for better performance

### Error Handling
- **Type-Safe Errors:** `CalcErr` enum instead of exceptions
- **Monadic Composition:** `EvalResult<T>` supports chaining operations
- **Context Preservation:** Error messages include expression context

### Parser Architecture
- **Shunting-Yard Algorithm:** Proper operator precedence handling
- **Recursive Descent:** Support for nested function calls
- **Token Streaming:** Efficient string-to-AST conversion

---

## 🔄 Version History

See [CHANGELOG.md](CHANGELOG.md) for detailed version history.

**Current Version: 2.5.0**
- ✅ Complete statistics engine with hypothesis testing
- ✅ Comprehensive unit conversion system (20+ unit types)
- ✅ ASCII function plotting capabilities
- ✅ Performance optimizations (arena allocation + memoization)
- ✅ Enhanced TUI with mode switching

---

## 🤝 Contributing

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feature/amazing-feature`
3. **Commit** changes: `git commit -m 'Add amazing feature'`
4. **Push** to branch: `git push origin feature/amazing-feature`
5. **Open** a Pull Request

### Development Guidelines
- Follow **C++20** best practices
- Maintain **exception-free** design patterns
- Add **unit tests** for new features
- Update **documentation** for API changes

---

## 📄 License

This project is licensed under the **MIT License** - see [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **FTXUI Library:** Modern terminal UI framework
- **C++20 Standards:** Concepts, ranges, and improved constexpr support
- **Mathematical Algorithms:** Numerical Recipes and NIST Statistical Handbook
- **Community:** C++ Core Guidelines and Modern C++ practices

---

*Built with precision, designed for extensibility, optimized for performance.*
