# C++ Dynamic Thread Safe Calculator

This project is a high-performance, thread-safe, and dynamically extensible mathematical expression engine written in Modern C++.

It evolved from a simple `switch`-statement alternative into a complete expression parser. It correctly handles operator precedence by implementing the **Shunting-Yard algorithm** and supports functions with a variable number of arguments (N-Arity) for maximum flexibility.

## 🌟 Core Architectural Features

This engine is built on modern C++ design principles:

* **Dynamic Dispatch (Strategy Pattern):** Operators are stored in an `std::map<std::string, ...>` using `std::function` objects. This allows new operators to be added at runtime without modifying the core logic.
* **Expression Parsing (Shunting-Yard):** The engine correctly parses infix expressions (e.g., `3 + 5 * 2`) into Reverse Polish Notation (RPN) before evaluation, respecting mathematical operator precedence.
* **Type-Safe Precedence:** Operator precedence is defined using a type-safe `enum class Precedence` instead of ambiguous "magic numbers".
* **N-Arity Function Support:** All function signatures accept an `std::vector<double>` of arguments. This allows the engine to seamlessly handle:
  * **Unary functions:** `sqrt 9`
  * **Binary functions:** `3 + 5`
  * **Future N-ary functions:** `sum(1, 2, 3)`
* **Exception-Free Error Handling:** Failed operations (e.g., divide by zero, `0^0`, domain errors) do not throw C++ exceptions. Instead, they return an `OperationResult` struct containing a `CalcErr` enum code for robust, high-performance error handling.
* **Thread-Safety (Read-Write Lock):** The operator maps (`ops_`, `unary_ops_`) are protected for concurrent access.
  * **Reads (`Evaluate`):** Use `std::shared_lock` for maximum read performance (multiple threads can evaluate simultaneously).
  * **Writes (Constructor/Register):** Use `std::lock_guard` for exclusive write access during initialization and runtime registration.
* **Runtime Extensibility:** The engine exposes public `RegisterOperator` and `RegisterUnaryOperator` methods, allowing new functions to be defined and added to the parser *after* compilation.

## 🛠️ Supported Operations

The engine supports a wide range of scientific and algebraic operations.

### Binary Operators (2 Arguments)

* **Arithmetic:** `+` (Add), `-` (Subtract), `*` (Multiply), `/` (Divide)
* **Power:** `^` (Exponentiation)
* **Modulus:** `%` (Modulus)

### Unary Operators (1 Argument)

* **Grouping:** `( ... )`
* **Basic:** `sqrt` (Square Root), `exp` (e^x)
* **Logarithmic:** `ln` (Natural log), `log` (Base 10 log), `lg` (Base 2 log)
* **Trigonometric (Input in Degrees):**
  * `sin`, `cos`, `tan`
  * `cot`, `sec`, `csc`
* **Inverse Trig (Output in Degrees):**
  * `arcsin`, `arccos`, `arctan`
  * `arccot`, `arcsec`, `arccsc`
* **Hyperbolic (Input is unitless):**
  * `sinh`, `cosh`, `tanh`
  * `coth`, `sech`, `csch`
* **Inverse Hyperbolic (Output is unitless):**
  * `arcsinh`, `arccosh`, `arctanh`
  * `arccoth`, `arcsech`, `arccsch`

## 🚀 How to Use

The engine is exposed via a single public method: `Evaluate(std::string expression)`.

### `main.cpp` Example

```cpp
#include <iostream>
#include <string>
#include "dynamic_calc.h" // Include the calculator library
#include <algorithm> // For std::max

int main() {
    Dynamic_calc calc_;
    
    // 1. Register a new operator at runtime (Example)
    std::cout << "Adding New Run-Time operator 'max' (example)\n";
    Operation max_op = [](const std::vector<double> &args) -> OperationResult {
        if (args.size() != 2) {
            return {std::nullopt, CalcErr::ArgumentMismatch};
        }
        double max_result = std::max(args[0], args[1]);
        return {std::optional<double>(max_result), CalcErr::None};
    };
    OperatorDetails max_details = {max_op, Precedence::AddSub};
    calc_.RegisterOperator("max", max_details);

    // 2. Get expression from user
    std::cout << "C++ Dynamic Calc Engine (v1.0)\n";
    // Note: The current tokenizer is whitespace-sensitive.
    std::cout << "Enter expression (e.g., 3 + 5 * ( sin 90 - 1 ) or 10 max 50)\n";
    std::cout << ">> ";
    
    std::string expression;
    std::getline(std::cin, expression);

    // 3. Call the main evaluation function
    auto evaluate_result = calc_.Evaluate(expression);
    
    // 4. Handle success or failure
    if (evaluate_result.err == CalcErr::None) {
        std::cout << "Result: " << evaluate_result.result.value() << "\n";
    } 
    else {
        std::cerr << "Error: ";
        switch (evaluate_result.err) {
            case CalcErr::DivideByZero:
                std::cerr << "Division by Zero!\n";
                break;
            case CalcName::IndeterminateResult:
                std::cerr << "Indeterminate Result (0/0, 0^0, etc.)!\n";
                break;
            case CalcErr::OperationNotFound:
                std::cerr << "Operation not found or invalid expression!\n";
                break;
            case CalcErr::ArgumentMismatch:
                std::cerr << "Expression Error (Missing number or operator)!\n";
                break;
            case CalcErr::NegativeRoot:
                std::cerr << "Cannot calculate square root of a negative number!\n";
                break;
            case CalcErr::DomainError:
                std::cerr << "Input is outside the function's domain (e.g., arcsin 2)!\n";
                break;
            default:
                std::cerr << "An unknown critical error occurred!\n";
                break;
        }
    }
    return 0;
}

🏗️ How to Build (CMake)

This is a standard CMake project. It can be opened directly in CLion, VS Code (with the CMake Tools extension), or Visual Studio 2019+.

CMakeLists.txt

Ensure your CMakeLists.txt file requires the C++17 standard (or newer).
CMake

cmake_minimum_required(VERSION 3.20)
project(DynamicCalc)

# Force the C++17 standard (required for std::optional, std::map<string, ...>, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where to find the header files
include_directories(.) 

# Create the executable
add_executable(DynamicCalc
        main.cpp
        dynamic_calc.cpp
        )