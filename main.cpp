#include <iostream>
#include <string>
#include "dynamic_calc.h"

int main() {
    std::string expression;
    Dynamic_calc calc_;

    std::cout << "C++ Dynamic Calc Engine (v1.0)\n";
    std::cout << "Enter expression (e.g., 3 + 5 * ( sin 90 - 1 ) )\n";
    std::cout << ">> ";

    std::getline(std::cin, expression);

    auto evaluate_result = calc_.Evaluate(expression);

    if (evaluate_result.err == CalcErr::None) {
        std::cout << "Result: " << evaluate_result.result.value() << "\n";
    } else {
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
            case CalcErr::DomainError:
                std::cerr << "Input is outside the function's domain (e.g., asin(2))!\n";
                break;
            default:
                std::cerr << "An unknown critical error occurred!\n";
                break;
        }
    }
    return 0;
}