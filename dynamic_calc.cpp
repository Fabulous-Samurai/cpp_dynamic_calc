// dynamic_calc.cpp
// This file implements the core calculator engine, which manages different calculation modes
// and delegates input parsing and execution to the appropriate parser.

#include "dynamic_calc.h"
#include "algebraic_parser.h"
#include "linear_system_parser.h"
#include <stdexcept>

// Constructor: Initializes the calculator engine and registers parsers for each mode.
CalcEngine::CalcEngine() {
    parsers_[CalcMode::Algebraic] = std::make_unique<AlgebraicParser>(); // Algebraic mode parser.
    parsers_[CalcMode::LinearSystem] = std::make_unique<LinearSystemParser>(); // Linear system mode parser.
}

// Sets the current calculation mode.
// @param mode: The calculation mode to set (e.g., Algebraic, LinearSystem).
void CalcEngine::SetMode(CalcMode mode) {
    current_mode_ = mode;
}

// Evaluates the given input string using the current calculation mode.
// @param input: The input string to evaluate.
// @return: The result of the evaluation as an EngineResult.
EngineResult CalcEngine::Evaluate(const std::string &input) {
    auto it = parsers_.find(current_mode_);
    if (it == parsers_.end()) {
        // If the current mode is not found, return an error.
        return {{}, {EngineErrorResult(CalcErr::OperationNotFound)}};
    }

    // Delegate parsing and execution to the appropriate parser.
    return it->second->ParseAndExecute(input);
}

// Registers a custom operator for the algebraic parser.
// @param op: The operator string (e.g., "+", "*").
// @param details: The details of the operator, including its implementation and precedence.
void CalcEngine::RegisterAlgebraicOperator(const std::string &op, const OperatorDetails &details) {
    auto it = parsers_.find(CalcMode::Algebraic);
    if (it != parsers_.end()) {
        // Cast the parser to AlgebraicParser and register the operator.
        AlgebraicParser *alg_parser = static_cast<AlgebraicParser *>(it->second.get());
        alg_parser->RegisterOperator(op, details);
    } else {
        // Throw an exception if the algebraic parser is not initialized.
        throw std::runtime_error("Algebraic parser not initialized.");
    }
}