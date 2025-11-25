#include "linear_system_parser.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <iostream>

LinearSystemParser::LinearSystemParser() {
    RegisterCommands();
}

void LinearSystemParser::RegisterCommands() {
    command_registry_.push_back({
        "qr", 
        [this](const std::string& s) { return HandleQR(s); },
        "Performs QR Decomposition"
    });
    
    command_registry_.push_back({
        "ortho", 
        [this](const std::string& s) { return HandleQR(s); },
        "Performs QR Decomposition"
    });

    command_registry_.push_back({
        "eigen", 
        [this](const std::string& s) { return HandleEigen(s); },
        "Computes Eigenvalues and Eigenvectors"
    });

    command_registry_.push_back({
        "cramer", 
        [this](const std::string& s) { return HandleCramer(s); },
        "Solves system using Cramer's Rule"
    });
}

EngineResult LinearSystemParser::ParseAndExecute(const std::string &input) {
    for (const auto& entry : command_registry_) {
        if (input.find(entry.command) == 0) {
            return entry.handler(input);
        }
    }
    return HandleDefaultSolve(input);
}

EngineResult LinearSystemParser::HandleQR(const std::string& input) {
    auto extract_matrix_string = [&input]() -> std::string {
        size_t matrix_start = input.find_first_of("0123456789-[");
        if (matrix_start == std::string::npos) return "";
        return input.substr(matrix_start);
    };

    std::string matrix_str = extract_matrix_string();
    if (matrix_str.empty()) return {{}, {LinAlgErr::ParseError}};

    Matrix A = ParseMatrixString(matrix_str);
    if (A.empty()) return {{}, {LinAlgErr::ParseError}};
    if (A.size() < A[0].size()) return {{}, {LinAlgErr::MatrixMismatch}};

    auto [Q, R] = GramSchmidt(A);
    if (Q.empty()) return {{}, {LinAlgErr::NoSolution}};
    
    return {EngineSuccessResult(Q), {}};
}

EngineResult LinearSystemParser::HandleEigen(const std::string& input) {
    auto extract_matrix_string = [&input]() -> std::string {
        size_t matrix_start = input.find_first_of("0123456789-[");
        if (matrix_start == std::string::npos) return "";
        return input.substr(matrix_start);
    };

    std::string matrix_str = extract_matrix_string();
    if (matrix_str.empty()) return {{}, {LinAlgErr::ParseError}};

    Matrix A = ParseMatrixString(matrix_str);
    if (A.empty() || A.size() != A[0].size()) return {{}, {LinAlgErr::MatrixMismatch}};

    auto [eigenValues, eigenVectors] = ComputeEigenvalues(A, 100);
    std::cout << "Eigenvalues found.\n";
    
    return {EngineSuccessResult(Vector(eigenValues)), {}};
}

EngineResult LinearSystemParser::HandleCramer(const std::string& input) {
    std::string equation = input.substr(6);
    std::vector<std::vector<double>> A;
    std::vector<double> b;

    if (!ParseLinearSystem(equation, A, b)) return {{}, {LinAlgErr::ParseError}};
    if (A.size() != A[0].size() || A.size() != b.size()) return {{}, {LinAlgErr::MatrixMismatch}};

    auto solution = CramersRule(A, b);
    if (solution.has_value()) return {solution.value(), {}};
    else return {{}, {LinAlgErr::NoSolution}};
}

EngineResult LinearSystemParser::HandleDefaultSolve(const std::string& input) {
    std::vector<std::vector<double>> A;
    std::vector<double> b;

    if (!ParseLinearSystem(input, A, b)) return {{}, {LinAlgErr::ParseError}};
    if (A.size() == 0 || A.size() != b.size()) return {{}, {LinAlgErr::MatrixMismatch}};

    LinAlgResult lin_res = solve_linear_system(A, b);
    if (lin_res.err == LinAlgErr::None) return {lin_res.solution.value(), {}};
    else return {{}, {lin_res.err}};
}

std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

Matrix LinearSystemParser::MultiplyMatrices(const Matrix& A, const Matrix& B){
    if(A.empty()||B.empty() || A[0].size() != B.size()) return{};
    int n = A.size();
    int m = B[0].size();
    int p = B.size();
    Matrix C(n,std::vector<double>(m,0.0));
    for(int i = 0; i<n; i++){
        for(int j =0 ; j<m; j++){
            for(int k = 0; k<p; k++) C[i][j] += A[i][k] * B[k][j]; 
        }
    }
    return C;
}

Matrix LinearSystemParser::CreateIdentityMatrix(int n) {
    Matrix I(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

std::vector<double> LinearSystemParser::GetDiagonal(const Matrix& A) {
    std::vector<double> diag;
    for (size_t i = 0; i < A.size(); ++i) diag.push_back(A[i][i]);
    return diag;
}

std::pair<std::vector<double>, Matrix> LinearSystemParser::ComputeEigenvalues(const Matrix& inputA ,int max_iterations){
    Matrix A = inputA;
    int n = A.size();
    Matrix EigenVectors = CreateIdentityMatrix(n);

    for(int k = 0 ; k<max_iterations; k++){
        auto [Q,R] = GramSchmidt(A);
        if(Q.empty()) break;
        A = MultiplyMatrices(R,Q);
        EigenVectors = MultiplyMatrices(EigenVectors,Q);
    }
    std::vector<double> eigenValues = GetDiagonal(A);
    return {eigenValues,EigenVectors};
}

bool LinearSystemParser::ParseLinearSystem(const std::string &input, std::vector<std::vector<double>> &A, std::vector<double> &b) {
    std::string processed_input;
    for (char c : input) if (c != ' ') processed_input += c;
    std::vector<std::string> equations;
    std::stringstream ss_eq(processed_input);
    std::string eq;
    while (std::getline(ss_eq, eq, ';')) equations.push_back(trim(eq));
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
        try { b[i] = std::stod(rhs); } catch (...) { return false; }
        std::string term;
        size_t pos = 0;
        while (pos < lhs.size()) {
            char sign = '+';
            if (lhs[pos] == '+' || lhs[pos] == '-') { sign = lhs[pos]; ++pos; }
            size_t start = pos;
            while (pos < lhs.size() && (std::isdigit(lhs[pos]) || lhs[pos] == '.' || std::isalpha(lhs[pos]))) ++pos;
            term = lhs.substr(start, pos - start);
            double coefficient = 1.0;
            char variable = '\0';
            size_t var_pos = term.find_first_of("abcdefghijklmnopqrstuvwxyz");
            if (var_pos != std::string::npos) {
                if (var_pos > 0) {
                    try { coefficient = std::stod(term.substr(0, var_pos)); } catch (...) { return false; }
                }
                variable = term[var_pos];
            } else {
                try { b[i] -= (sign == '+' ? 1 : -1) * std::stod(term); } catch (...) { return false; }
                continue;
            }
            if (sign == '-') coefficient = -coefficient;
            if (var_to_index.find(variable) == var_to_index.end()) var_to_index[variable] = var_count++;
            int col = var_to_index[variable];
            A[i][col] += coefficient;
        }
    }
    if (var_count != N) return false;
    return true;
}

double LinearSystemParser::DotProduct(const std::vector<double> &v1, const std::vector<double> &v2) {
    if (v1.size() != v2.size()) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < v1.size(); i++) sum += v1[i] * v2[i];
    return sum;
}

double LinearSystemParser::VectorNorm(const std::vector<double> &v) {
    return std::sqrt(DotProduct(v, v));
}

std::vector<double> LinearSystemParser::VectorScale(const std::vector<double> &v, const double scalar) {
    std::vector<double> result = v;
    for (double &val : result) val *= scalar;
    return result;
}

std::vector<double> LinearSystemParser::VectorSub(const std::vector<double> &v1, const std::vector<double> &v2) {
    if (v1.size() != v2.size()) return {};
    std::vector<double> result = v1;
    for (size_t i = 0; i < result.size(); i++) result[i] -= v2[i];
    return result;
}

Matrix LinearSystemParser::GetMinor(const Matrix &A, int row, int col) {
    int n = A.size();
    Matrix minor;
    for (int i = 0; i < n; i++) {
        if (i == row) continue;
        std::vector<double> minor_row;
        for (int j = 0; j < n; j++) {
            if (j == col) continue;
            minor_row.push_back(A[i][j]);
        }
        minor.push_back(minor_row);
    }
    return minor;
}

double LinearSystemParser::Determinant(const Matrix &A) {
    int n = A.size();
    if (n == 1) return A[0][0];
    double det = 0.0;
    for (int j = 0; j < n; j++) {
        Matrix minor = GetMinor(A, 0, j);
        double minor_det = Determinant(minor);
        det += (j % 2 == 0 ? 1 : -1) * A[0][j] * minor_det;
    }
    return det;
}

Matrix LinearSystemParser::Transpose(const Matrix &A) {
    if (A.empty()) return {};
    int N = A.size();
    int M = A[0].size();
    Matrix T(M, std::vector<double>(N));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) T[j][i] = A[i][j];
    }
    return T;
}

std::pair<Matrix, Matrix> LinearSystemParser::GramSchmidt(const Matrix &A) {
    Matrix A_cols = Transpose(A);
    if (A_cols.empty()) return {{}, {}};
    int N = A_cols.size();
    Matrix Q_cols = A_cols;
    Matrix R(N, std::vector<double>(N, 0.0));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < i; j++) {
            R[j][i] = DotProduct(Q_cols[j], A_cols[i]);
            std::vector<double> projection = VectorScale(Q_cols[j], R[j][i]);
            Q_cols[i] = VectorSub(Q_cols[i], projection);
        }
        R[i][i] = VectorNorm(Q_cols[i]);
        if (std::abs(R[i][i]) > 1e-9) {
            Q_cols[i] = VectorScale(Q_cols[i], (1.0 / R[i][i]));
        } else {
            return {{}, {}};
        }
    }
    Matrix Q = Transpose(Q_cols);
    return {Q, R};
}

LinAlgResult LinearSystemParser::solve_linear_system(const std::vector<std::vector<double>> &A, const std::vector<double> &b) {
    int N = A.size();
    if (N == 0 || A[0].size() != N || b.size() != N) return {std::nullopt, LinAlgErr::MatrixMismatch};
    std::vector<std::vector<double>> M(N, std::vector<double>(N + 1));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; ++j) M[i][j] = A[i][j];
        M[i][N] = b[i];
    }
    for (int i = 0; i < N; i++) {
        int max_row = i;
        for (int k = i + 1; k < N; k++) {
            if (std::abs(M[k][i]) > std::abs(M[max_row][i])) max_row = k;
        }
        std::swap(M[i], M[max_row]);
        if (std::abs(M[i][i]) < 1e-9) return {std::nullopt, LinAlgErr::NoSolution};
        for (int j = i + 1; j <= N; j++) M[i][j] /= M[i][i];
        for (int k = 0; k < N; k++) {
            if (k != i) {
                double factor = M[k][i];
                for (int j = i; j <= N; j++) M[k][j] -= factor * M[i][j];
            }
        }
    }
    std::vector<double> solution(N);
    for (int i = 0; i < N; i++) solution[i] = M[i][N];
    return {std::optional<std::vector<double>>(solution), LinAlgErr::None};
}

std::optional<std::vector<double>> LinearSystemParser::CramersRule(const Matrix &A, const std::vector<double> &b) {
    int n = A.size();
    if (n != b.size()) return std::nullopt;
    double detA = Determinant(A);
    if (isCloseToZero(detA)) return std::nullopt;
    std::vector<double> solution(n);
    for (int i = 0; i < n; ++i) {
        Matrix Ai = A;
        for (int j = 0; j < n; ++j) Ai[j][i] = b[j];
        double detAi = Determinant(Ai);
        solution[i] = detAi / detA;
    }
    return solution;
}

bool isCloseToZero(double value, double epsilon) {
    return std::abs(value) < epsilon;
}

bool isValidNumber(const std::string &str) {
    try { std::stod(str); return true; } catch (...) { return false; }
}

Matrix LinearSystemParser::ParseMatrixString(const std::string &input) {
    Matrix result;
    std::vector<double> current_row;
    std::string num_str;
    if (input.empty()) return {};
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        if (std::isdigit(c) || c == '.' || c == '-' || c == '+') {
            num_str += c;
        } else {
            if (!num_str.empty()) {
                try { current_row.push_back(std::stod(num_str)); } catch (...) {}
                num_str.clear();
            }
            if (c == ';' || c == ']') {
                if (!current_row.empty()) {
                    result.push_back(current_row);
                    current_row.clear();
                }
            }
        }
    }
    if (!num_str.empty()) {
        try { current_row.push_back(std::stod(num_str)); } catch (...) {}
        if (!current_row.empty()) result.push_back(current_row);
    }
    if(result.empty()) return {};
    size_t cols = result[0].size();
    for (const auto &row : result) {
        if (row.size() != cols) return {}; 
    }
    return result;
}