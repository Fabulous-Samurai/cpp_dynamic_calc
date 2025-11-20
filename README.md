

-----

# C++ Dynamic Thread-Safe Calculation Engine

> **From a simple `switch` statement to a multi-paradigm mathematical engine.**

This project represents a deep dive into modern C++ architecture, evolving from a basic calculator concept into a robust, thread-safe, and dynamically extensible analysis engine. It demonstrates the transition from procedural code to advanced design patterns like **Strategy**, **Dependency Injection**, and **Data-Oriented Design**.

## 🚀 Evolution & Architecture

The engine was built in distinct evolutionary stages, each solving a critical architectural challenge:

### Phase I: The Dynamic Core (OCP)

  * **Challenge:** Traditional calculators use rigid `switch` statements that require recompilation to add new features.
  * **Solution:** Implemented the **Open/Closed Principle** using a **Dynamic Dispatch** system. Operators are stored in `std::map` structures as `std::function` objects, allowing new operations (binary or unary) to be registered at **runtime** without modifying the source code.

### Phase II: Expression Intelligence (Shunting-Yard)

  * **Challenge:** Simple command execution cannot handle operator precedence (e.g., `3 + 5 * 2`).
  * **Solution:** Integrated a custom implementation of Dijkstra's **Shunting-Yard Algorithm**. The engine now tokenizes strings, respects mathematical precedence (PEMDAS), handles parentheses, and computes results using a **Reverse Polish Notation (RPN)** stack machine.

### Phase III: Professional Stability (SOLID & Thread-Safety)

  * **Challenge:** As features grew, the codebase became monolithic and unsafe for concurrent use.
  * **Solution:**
      * **Thread Safety:** Implemented `std::shared_mutex` for a **Readers-Writer Lock** model, allowing multiple threads to evaluate expressions simultaneously while ensuring exclusive access during operator registration.
      * **Exception-Free Design:** Replaced costly C++ exceptions with a `std::variant`-based `EngineResult` type, providing deterministic and high-performance error handling.
      * **SOLID Refactoring:** The monolithic class was refactored into specialized **Parser Strategies** (`AlgebraicParser`, `LinearSystemParser`) managed by a central `CalcEngine` (Context), adhering to the **Single Responsibility Principle**.

### Phase IV: Scientific & Linear Algebra Expansion

  * **Challenge:** A calculator is limited without advanced math and system solving capabilities.
  * **Solution:**
      * **Advanced Math:** Added support for **Trigonometric** (sin, cos, tan), **Inverse Trigonometric** (arcsin, arccos), **Hyperbolic** (sinh, cosh), and **Logarithmic** (log, ln, lg) functions.
      * **Linear Systems:** Integrated a **Gauss-Jordan Elimination** solver capable of parsing and solving $N \times N$ linear equation systems (e.g., `2x + y = 5`).

-----

## 🛠️ Features

  * **Algebraic Engine:**
      * Binary Operators: `+`, `-`, `*`, `/`, `^`, `%`
      * Unary Functions: `sqrt`, `abs`, `exp`
      * Trigonometry: `sin`, `cos`, `tan`, `cot`, `sec`, `csc` (Degrees)
      * Inverse Trig: `arcsin`, `arccos`, `arctan`, etc.
      * Logarithms: `log` (base 10), `ln` (base e), `lg` (base 2)
  * **Linear Algebra Engine:**
      * Parses string equations (e.g., `"2x + 3y = 10; x - y = 5"`)
      * Solves for $N$ variables using optimized matrix algorithms.
  * **System Features:**
      * **Zero-Overhead Error Handling:** Uses `std::optional` and `std::variant`.
      * **Hot-Swappable Modes:** Switch between `Algebraic` and `LinearSystem` modes at runtime.

## 💻 Usage Example

```cpp
#include <iostream>
#include "calc_engine.h"

int main() {
    CalcEngine engine;

    // 1. Algebraic Mode (Default)
    auto result = engine.Evaluate("3 + 5 * ( sin 90 - 1 )");
    // Output: Result: 3

    // 2. Linear System Mode
    engine.SetMode(CalcMode::LinearSystem);
    auto sys_result = engine.Evaluate("2x + y = 5; x - y = 1");
    // Output: Result: [ 2, 1 ]  (where x=2, y=1)
    
    return 0;
}
```

## 🏗️ Build

This project uses **CMake** for cross-platform compatibility.

```bash
mkdir build && cd build
cmake ..
make
./DynamicCalc
```

-----

*This project is a testament to clean code, architectural discipline, and the power of Modern C++.*
