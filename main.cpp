#include <iostream>
#include <string>
#include "dynamic_calc.h"

void PrintResult(const EngineResult& result) {
    if (result.error.has_value() && result.result.has_value()) {
        std::cerr << "Error: Both error and result are set, which is invalid.\n";
    } else if (result.error.has_value()) {
        std::cerr << "Error: ";
        std::visit([](auto&& err) {
            using T = std::decay_t<decltype(err)>;
            if constexpr (std::is_same_v<T, CalcErr>) {
                switch (err) {
                    case CalcErr::DivideByZero: std::cerr << "Division by 0 Error!\n"; break;
                    case CalcErr::IndeterminateResult: std::cerr << "Indeterminate Result (0/0, vb.)!\n"; break;
                    case CalcErr::OperationNotFound: std::cerr << "Operator Can't Find!\n"; break;
                    case CalcErr::ArgumentMismatch: std::cerr << "Expression Error (Incomplete Number or Operator)!\n"; break;
                    case CalcErr::NegativeRoot: std::cerr << "Negative Root Error!\n"; break;
                    case CalcErr::DomainError: std::cerr << "Domain Error (e.g.: arcsin(2))!\n"; break;
                    default: std::cerr << "Unknown Algebraic Error!\n"; break;
                }
            } else if constexpr (std::is_same_v<T, LinAlgErr>) {
                switch (err) {
                    case LinAlgErr::NoSolution: std::cerr << "No Solution!\n"; break;
                    case LinAlgErr::InfiniteSolutions: std::cerr << "Infinite Solutions!\n"; break;
                    case LinAlgErr::ParseError: std::cerr << "Equation can't be Separated!\n"; break;
                    default: std::cerr << "Unknown Linear Algebraic Error!\n"; break;
                }
            }
        }, result.error.value());

    } else if (result.result.has_value()) {
        std::cout << "Result: ";
        std::visit([](auto&& res) {
            using T = std::decay_t<decltype(res)>;
            if constexpr (std::is_same_v<T, double>) {
                std::cout << res << "\n";
            } else if constexpr (std::is_same_v<T, std::vector<double>>) {
                std::cout << "[ ";
                for (double d : res) {
                    std::cout << d << " ";
                }
                std::cout << "]\n";
            } else if constexpr (std::is_same_v<T, Matrix>) {
                std::cout << "[\n";
                for (const auto& row : res) {
                    std::cout << "  [ ";
                    for (double val : row) {
                        std::cout << val << " ";
                    }
                    std::cout << "]\n";
                }
                std::cout << "]\n";
            }
        }, result.result.value());
    }
}

int main() {
    std::string expression;
    CalcEngine calc_;

    std::cout << "C++ Calc Engine (v2.0 - Refactored)\n";
    std::cout << "Commands: mode algebraic | mode linear | exit\n";

    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, expression);

        if (expression == "exit") {
            break;
        } else if (expression == "mode algebraic") {
            calc_.SetMode(CalcMode::Algebraic);
            std::cout << "Mode switched to Algebraic.\n";
            continue;
        } else if (expression == "mode linear") {
            calc_.SetMode(CalcMode::LinearSystem);
            std::cout << "Mode switched to Linear System.\n";
            continue;
        }

        auto result = calc_.Evaluate(expression);
        PrintResult(result);
    }

    return 0;
}