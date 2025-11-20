#pragma once
#include "IParser.h"
#include "dynamic_calc_types.h"
#include <vector>
#include <string>

struct LinAlgResult {
    std::optional<std::vector<double>> solution;
    LinAlgErr err;
};

class LinearSystemParser : public IParser {
public:
    EngineResult ParseAndExecute(const std::string& input) override;

private:
    LinAlgResult solve_linear_system(const std::vector<std::vector<double>>& A, const std::vector<double>& b);
    bool ParseLinearSystem(const std::string& input, std::vector<std::vector<double>>& A, std::vector<double>& b);

    Matrix GetMinor(const Matrix& A, int row, int col);
    double Determinant(const Matrix& A);
    std::optional<std::vector<double>> CramersRule(const Matrix& A, const std::vector<double>& b);
};

bool isCloseToZero(double value, double epsilon = 1e-9);
bool isValidNumber(const std::string& str);