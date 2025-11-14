# C++ Dynamic Thread Safe Calculator

This project is a high-performance, thread-safe, and dynamically extensible mathematical expression engine written in Modern C++.

It evolved from a simple `switch`-statement alternative into a complete expression parser. It correctly handles operator precedence by implementing the **Shunting-Yard algorithm** and supports functions with a variable number of arguments (N-Arity) for maximum flexibility.

## 🌟 Core Architectural Features

This engine is built on modern C++ design principles:

* **Dynamic Dispatch (Strategy Pattern):** Operators are stored in an `std::map` as `std::function` objects. This allows new operators to be added at runtime without modifying the core logic.
* **Expression Parsing (Shunting-Yard):** The engine correctly parses infix expressions (e.g., `3 + 5 * 2`) into Reverse Polish Notation (RPN) before evaluation, respecting mathematical operator precedence.
* **Type-Safe Precedence:** Operator precedence is defined using a type-safe `enum class Precedence` instead of ambiguous "magic numbers" (like 1, 2, 3).
* **N-Arity Function Support:** All function signatures accept an `std::vector<double>` of arguments. This allows the engine to seamlessly handle:
    * **Unary functions:** `s(9)` (sqrt)
    * **Binary functions:** `3 + 5`
    * **Future N-ary functions:** `sum(1, 2, 3)`
* **Exception-Free Error Handling:** Failed operations (e.g., divide by zero, `0^0`, `0 % 0`) do not throw C++ exceptions. Instead, they return an `OperationResult` struct containing a `CalcErr` error code for robust, high-performance error handling.
* **Thread-Safety (Read-Write Lock):** The operator maps (`ops_`, `unary_ops_`) are protected for concurrent access.
    * **Reads (`Evaluate`):** Use `std::shared_lock` for maximum read performance (multiple threads can evaluate simultaneously).
    * **Writes (Constructor):** Use `std::lock_guard` for exclusive write access during initialization.

## 🛠️ Supported Operations

* **Binary Operators:** `+`, `-`, `*`, `/`, `^` (power), `%` (modulus)
* **Unary Operators:** `s` (square root)
* **Grouping:** `( ... )`

## 🚀 How to Use

The engine is exposed via a single public method: `Evaluate(std::string expression)`.

### `main.cpp` Example

```cpp
#include <iostream>
#include <string>
#include "dynamic_calc.h" // Include the calculator library

int main() {
    std::string expression;
    Dynamic_calc calc_;
    
    std::cout << "C++ Dynamic Calc Engine (v1.0)\n";
    // Note: The current tokenizer is whitespace-sensitive.
    std::cout << "Enter expression (e.g., 3 + 5 * ( s 9 - 1 ) )\n";
    
    // Use std::getline to read the full line with spaces
    std::getline(std::cin, expression);

    // Call the main evaluation function
    auto evaluate_result = calc_.Evaluate(expression);
    
    // 1. Check for success
    if (evaluate_result.err == CalcErr::None) {
        std::cout << "Result: " << evaluate_result.result.value() << "\n";
    } 
    // 2. Handle errors
    else {
        std::cerr << "Error: ";
        switch (evaluate_result.err) {
            case CalcErr::DivideByZero:
                std::cerr << "Division by Zero!\n";
                break;
            case CalcErr::IndeterminateResult:
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
            default:
                std::cerr << "An unknown critical error occurred!\n";
                break;
        }
    }
    return 0;
}
