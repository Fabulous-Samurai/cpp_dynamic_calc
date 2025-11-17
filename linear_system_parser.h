#pragma once
#include "IParser.h"
#include "dynamic_calc_types.h"


struct LinAlgResult {
    std::optional<std::vector<double>> solution;
    LinAlgErr err;
};
#include <vector>
#include <string>

class LinearSystemParser : public IParser {
public:
    LinearSystemParser() = default;
    EngineResult ParseAndExecute(const std::string& input) override;

private:
    LinAlgResult solve_linear_system(const std::vector<std::vector<double>>& A, const std::vector<double>& b);
    bool ParseLinearSystem(const std::string& input, std::vector<std::vector<double>>& A, std::vector<double>& b);
};