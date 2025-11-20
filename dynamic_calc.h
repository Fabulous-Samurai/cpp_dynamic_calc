#pragma once

#include "IParser.h"
#include "dynamic_calc_types.h"
#include <map>
#include <memory>
#include <string>

enum class CalcMode {
    Algebraic,
    LinearSystem
};

class CalcEngine {
private:
    std::map<CalcMode, std::unique_ptr<IParser>> parsers_;
    CalcMode current_mode_ = CalcMode::Algebraic;

public:
    CalcEngine();

    void SetMode(CalcMode mode);
    EngineResult Evaluate(const std::string& input);

    void RegisterAlgebraicOperator(const std::string& op, const OperatorDetails& details);
};