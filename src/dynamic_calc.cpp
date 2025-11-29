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
   return EvaluateWithContext(input,{});
}

EngineResult CalcEngine::EvaluateWithContext(const std::string& input,const std::map<std::string,double>& context){
    auto it = parsers_.find(current_mode_);
    if(it == parsers_.end()){
        return {{},{EngineErrorResult(CalcErr::OperationNotFound)}};
    }

    if(current_mode_== CalcMode::Algebraic){
        AlgebraicParser* alg_parser = static_cast<AlgebraicParser*>(it->second.get());
        return alg_parser->ParseAndExecuteWithContext(input,context);
    }
    
    return it->second->ParseAndExecute(input);
}
