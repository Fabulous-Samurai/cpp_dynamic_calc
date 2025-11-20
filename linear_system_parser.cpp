#include "linear_system_parser.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <iostream>

EngineResult LinearSystemParser::ParseAndExecute(const std::string &input)
{
    if (input.find("cramer") == 0) { // Check if the input starts with "cramer"
        std::string equation = input.substr(6); // Extract the equation after "cramer"
        std::vector<std::vector<double>> A;
        std::vector<double> b;

        std::cout << "Parsing equations for Cramer's Rule: " << equation << std::endl;

        bool parse_success = ParseLinearSystem(equation, A, b);
        if (!parse_success) {
            std::cout << "Error: Failed to parse the input." << std::endl;
            return {{}, {LinAlgErr::ParseError}};
        }

        std::cout << "Parsed matrix A and vector b successfully." << std::endl;

        auto solution = CramersRule(A, b);
        if (solution.has_value()) {
            return {solution.value(), {}};
        } else {
            std::cout << "Error: Cramer's Rule failed." << std::endl;
            return {{}, {LinAlgErr::NoSolution}};
        }
    }

    // Default behavior for other inputs
    std::vector<std::vector<double>> A;
    std::vector<double> b;

    bool parse_success = ParseLinearSystem(input, A, b);
    if (!parse_success)
    {
        return {{}, {LinAlgErr::ParseError}};
    }

    LinAlgResult lin_res = solve_linear_system(A, b);

    if (lin_res.err == LinAlgErr::None)
    {
        return {lin_res.solution.value(), {}};
    }
    else
    {
        return {{}, {lin_res.err}};
    }
}

std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

bool LinearSystemParser::ParseLinearSystem(const std::string &input,
                                           std::vector<std::vector<double>> &A,
                                           std::vector<double> &b) {
    std::cout << "Input to ParseLinearSystem: " << input << std::endl;

    // Preprocess the input to remove spaces
    std::string processed_input;
    for (char c : input) {
        if (c != ' ') {
            processed_input += c;
        }
    }

    std::vector<std::string> equations;
    std::stringstream ss_eq(processed_input);
    std::string eq;

    while (std::getline(ss_eq, eq, ';')) {
        equations.push_back(trim(eq));
    }

    int N = equations.size();
    if (N == 0) {
        std::cout << "Error: No equations found." << std::endl;
        return false;
    }

    A.assign(N, std::vector<double>(N, 0.0));
    b.assign(N, 0.0);
    std::map<char, int> var_to_index;
    int var_count = 0;

    for (int i = 0; i < N; ++i) {
        std::stringstream ss_eq_part(equations[i]);
        std::string part;

        std::getline(ss_eq_part, part, '=');
        std::string lhs = trim(part);
        if (!std::getline(ss_eq_part, part, '=')) {
            std::cout << "Error: Missing '=' in equation: " << equations[i] << std::endl;
            return false;
        }
        std::string rhs = trim(part);

        try {
            b[i] = std::stod(rhs);
        } catch (...) {
            std::cout << "Error: Invalid right-hand side in equation: " << equations[i] << std::endl;
            return false;
        }

        // Parse the left-hand side
        std::string term;
        size_t pos = 0;
        while (pos < lhs.size()) {
            // Extract the sign (+ or -)
            char sign = '+';
            if (lhs[pos] == '+' || lhs[pos] == '-') {
                sign = lhs[pos];
                ++pos;
            }

            // Extract the term (e.g., 2x, 3y)
            size_t start = pos;
            while (pos < lhs.size() && (std::isdigit(lhs[pos]) || lhs[pos] == '.' || std::isalpha(lhs[pos]))) {
                ++pos;
            }
            term = lhs.substr(start, pos - start);

            // Parse the coefficient and variable
            double coefficient = 1.0;
            char variable = '\0';
            size_t var_pos = term.find_first_of("abcdefghijklmnopqrstuvwxyz");
            if (var_pos != std::string::npos) {
                if (var_pos > 0) {
                    try {
                        coefficient = std::stod(term.substr(0, var_pos));
                    } catch (...) {
                        std::cout << "Error: Invalid coefficient in term: " << term << std::endl;
                        return false;
                    }
                }
                variable = term[var_pos];
            } else {
                try {
                    b[i] -= (sign == '+' ? 1 : -1) * std::stod(term);
                } catch (...) {
                    std::cout << "Error: Invalid constant term: " << term << std::endl;
                    return false;
                }
                continue;
            }

            // Apply the sign to the coefficient
            if (sign == '-') {
                coefficient = -coefficient;
            }

            // Map the variable to a column in the matrix
            if (var_to_index.find(variable) == var_to_index.end()) {
                var_to_index[variable] = var_count++;
            }

            int col = var_to_index[variable];
            A[i][col] += coefficient;
        }
    }

    if (var_count != N) {
        std::cout << "Error: Number of variables does not match the number of equations." << std::endl;
        return false;
    }

    std::cout << "Matrix A: " << std::endl;
    for (const auto &row : A) {
        for (double val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Vector b: ";
    for (double val : b) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return true;
}

Matrix LinearSystemParser::GetMinor(const Matrix& A , int row , int col){
    int n = A.size();
    Matrix minor;

    for(int i = 0 ; i < n ; i++){
        if(i==row) continue;
        std::vector<double> minor_row;
        for (int j = 0; j < n; j++)
        {
        if(j==col) continue;
        minor_row.push_back(A[i][j]);
        }
        minor.push_back(minor_row);
    }
    return minor;
}

double LinearSystemParser::Determinant(const Matrix& A) {
    int n = A.size();
    if (n == 1) return A[0][0];
    double det = 0.0;
    for (int j = 0; j < n; j++) {
        Matrix minor = GetMinor(A, 0, j);
        double minor_det = Determinant(minor); 
        det += (j % 2 == 0 ? 1 : -1) * A[0][j] * minor_det;

        std::cout << "Determinant step: A[0][" << j << "] = " << A[0][j]
                  << ", minor determinant = " << minor_det
                  << ", partial determinant = " << det << std::endl;
    }

    return det;
}

LinAlgResult LinearSystemParser::solve_linear_system(const std::vector<std::vector<double>> &A, const std::vector<double> &b)
{
    std::cout << "Solving linear system with matrix A and vector b." << std::endl;
    std::cout << "Matrix A: \n";
    for (const auto &row : A)
    {
        for (double val : row)
        {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Vector b: ";
    for (double val : b)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    int N = A.size();
    if (N == 0 || A[0].size() != N || b.size() != N)
    {
        std::cout << "Error: Matrix dimensions do not match." << std::endl;
        return {std::nullopt, LinAlgErr::MatrixMismatch};
    }
    std::vector<std::vector<double>> M(N, std::vector<double>(N + 1));
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; ++j)
        {
            M[i][j] = A[i][j];
        }
        M[i][N] = b[i];
    }

    
    for (int i = 0; i < N; i++)
    {
        int max_row = i;
        for (int k = i + 1; k < N; k++)
        {
            if (std::abs(M[k][i]) > std::abs(M[max_row][i]))
            {
                max_row = k;
            }
        }
        std::swap(M[i], M[max_row]);

        if (std::abs(M[i][i]) < 1e-9)
        {
            std::cout << "Error: Singular matrix detected. No unique solution." << std::endl;
            return {std::nullopt, LinAlgErr::NoSolution};
        }

        for (int j = i + 1; j <= N; j++)
        {
            M[i][j] /= M[i][i];
        }

        for (int k = 0; k < N; k++)
        {
            if (k != i)
            {
                double factor = M[k][i];
                for (int j = i; j <= N; j++)
                {
                    M[k][j] -= factor * M[i][j];
                }
            }
        }
    }

    std::vector<double> solution(N);
    for (int i = 0; i < N; i++)
    {
        solution[i] = M[i][N];
    }

    std::cout << "Solution vector: ";
    for (double val : solution)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return {std::optional<std::vector<double>>(solution), LinAlgErr::None};
}

std::optional<std::vector<double>> LinearSystemParser::CramersRule(const Matrix& A, const std::vector<double>& b) {
    int n = A.size();
    if (n != b.size()) {
        std::cout << "Error: Matrix and vector dimensions do not match." << std::endl;
        return std::nullopt;
    }

    double detA = Determinant(A);
    if (isCloseToZero(detA)) {
        std::cout << "Error: Determinant of A is zero. No unique solution." << std::endl;
        return std::nullopt;
    }

    std::vector<double> solution(n);
    for (int i = 0; i < n; ++i) {
        Matrix Ai = A;
        for (int j = 0; j < n; ++j) {
            Ai[j][i] = b[j];
        }

        double detAi = Determinant(Ai);
        solution[i] = detAi / detA;

    
        std::cout << "Cramer's Rule step: det(Ai) = " << detAi
                  << ", det(A) = " << detA
                  << ", solution[" << i << "] = " << solution[i] << std::endl;
    }

    return solution;
}

bool isCloseToZero(double value, double epsilon) {
    return std::abs(value) < epsilon;
}

bool isValidNumber(const std::string& str) {
    try {
        std::stod(str);
        return true;
    } catch (...) {
        return false;
    }
}


