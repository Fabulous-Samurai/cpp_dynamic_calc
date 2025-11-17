#include "linear_system_parser.h"
#include "dynamic_calc_types.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <iostream>

EngineResult LinearSystemParser::ParseAndExecute(const std::string& input) {
    std::vector<std::vector<double>> A;
    std::vector<double> b;

    bool parse_success = ParseLinearSystem(input, A, b);
    if (!parse_success) {
        return {{}, {LinAlgErr::ParseError}};
    }

    LinAlgResult lin_res = solve_linear_system(A, b);

    if (lin_res.err == LinAlgErr::None) {
        return {lin_res.solution.value(), {}};
    } else {
        return {{}, {lin_res.err}};
    }
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

bool LinearSystemParser::ParseLinearSystem(const std::string& input,
                                           std::vector<std::vector<double>>& A,
                                           std::vector<double>& b)
{
    std::vector<std::string> equations;
    std::stringstream ss_eq(input);
    std::string eq;

    while (std::getline(ss_eq, eq, ';')) {
        equations.push_back(trim(eq));
    }

    int N = equations.size();
    if (N == 0) return false;

    A.assign(N, std::vector<double>(N, 0.0));
    b.assign(N, 0.0);
    std::map<char, int> var_to_index;
    int var_count = 0;

    for (int i = 0; i < N; ++i) {
        std::stringstream ss_eq_part(equations[i]);
        std::string part;
        std::getline(ss_eq_part, part, '=');
        std::string lhs = trim(part);
        if (!std::getline(ss_eq_part, part, '=')) return false;
        std::string rhs = trim(part);

        try {
            b[i] = std::stod(rhs);
        } catch (...) {
            return false;
        }

        std::stringstream ss_lhs(lhs);
        std::string term;

        while (ss_lhs >> term) {
            if (term == "+") continue;

            double coefficient = 1.0;
            char var_name = 0;
            std::string coeff_str;

            size_t var_pos = std::string::npos;
            for (size_t j = 0; j < term.length(); ++j) {
                if (isalpha(term[j])) {
                    var_pos = j;
                    var_name = term[j];
                    break;
                }
            }

            if (var_pos == std::string::npos) continue;

            coeff_str = term.substr(0, var_pos);

            if (coeff_str.empty() || coeff_str == "+") {
                coefficient = 1.0;
            } else if (coeff_str == "-") {
                coefficient = -1.0;
            } else {
                try {
                    coefficient = std::stod(coeff_str);
                } catch (...) {
                    return false;
                }
            }

            if (var_to_index.find(var_name) == var_to_index.end()) {
                var_to_index[var_name] = var_count++;
            }

            if (var_to_index[var_name] >= N) return false;

            A[i][var_to_index[var_name]] = coefficient;
        }
    }

    return var_count == N;
}

LinAlgResult LinearSystemParser::solve_linear_system(const std::vector<std::vector<double>>& A, const std::vector<double>& b) {
    int N = A.size();
    if (N == 0 || A[0].size() != N || b.size() != N) {
        return {std::nullopt, LinAlgErr::MatrixMismatch};
    }

    std::vector<std::vector<double>> M(N, std::vector<double>(N + 1));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; ++j) {
            M[i][j] = A[i][j];
        }
        M[i][N] = b[i];
    }

    for (int i = 0; i < N; i++) {
        int max_row = i;
        for (int k = i + 1; k < N; k++) {
            if (std::abs(M[k][i]) > std::abs(M[max_row][i])) {
                max_row = k;
            }
        }
        std::swap(M[i], M[max_row]);

        if (std::abs(M[i][i]) < 1e-9) {
            if (std::abs(M[i][N]) > 1e-9) {
                return {std::nullopt, LinAlgErr::NoSolution};
            }
            return {std::nullopt, LinAlgErr::InfiniteSolutions};
        }

        double pivot = M[i][i];
        for (int j = i; j <= N; j++) {
            M[i][j] /= pivot;
        }

        for (int k = 0; k < N; k++) {
            if (k != i) {
                double factor = M[k][i];
                for (int j = i; j <= N; j++) {
                    M[k][j] -= factor * M[i][j];
                }
            }
        }
    }

    std::vector<double> solution(N);
    for (int i = 0; i < N; i++) {
        solution[i] = M[i][N];
    }
    return {solution, LinAlgErr::None};
}