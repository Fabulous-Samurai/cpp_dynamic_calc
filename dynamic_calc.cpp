#include "dynamic_calc.h"
#include "algebraic_parser.h"
#include "linear_system_parser.h"
#include <stdexcept>

CalcEngine::CalcEngine() {
    parsers_[CalcMode::Algebraic] = std::make_unique<AlgebraicParser>();
    parsers_[CalcMode::LinearSystem] = std::make_unique<LinearSystemParser>();
}

void CalcEngine::SetMode(CalcMode mode) {
    current_mode_ = mode;
}

EngineResult CalcEngine::Evaluate(const std::string &input) {
    auto it = parsers_.find(current_mode_);
    if (it == parsers_.end()) {
        return {{},
                {EngineErrorResult(CalcErr::OperationNotFound)}};
    }

    return it->second->ParseAndExecute(input);
}

void CalcEngine::RegisterAlgebraicOperator(const std::string &op, const OperatorDetails &details) {
    auto it = parsers_.find(CalcMode::Algebraic);
    if (it != parsers_.end()) {
        AlgebraicParser *alg_parser = static_cast<AlgebraicParser *>(it->second.get());
        alg_parser->RegisterOperator(op, details);
    } else {
        throw std::runtime_error("Algebraic parser not initialized.");
    }
}