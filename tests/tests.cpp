#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include <iomanip>
#include <functional>

// Proje dosyalarını dahil ediyoruz
#include "dynamic_calc.h"
#include "string_helpers.h"

// =========================================================================
// MICRO TEST FRAMEWORK (Dependency-Free)
// =========================================================================
int g_tests_passed = 0;
int g_tests_failed = 0;

#define ASSERT_EQ(val1, val2) \
    if ((val1) == (val2)) { g_tests_passed++; } \
    else { \
        std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << " Expected " << (val1) << " but got " << (val2) << "\n"; \
        g_tests_failed++; \
    }

#define ASSERT_NEAR(val1, val2, tol) \
    if (std::abs((val1) - (val2)) < (tol)) { g_tests_passed++; } \
    else { \
        std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << " Expected " << (val1) << " near " << (val2) << " (diff: " << std::abs((val1)-(val2)) << ")\n"; \
        g_tests_failed++; \
    }

#define RUN_TEST(test_func) \
    std::cout << "[RUNNING] " << #test_func << "...\n"; \
    try { \
        test_func(); \
        std::cout << "[   OK  ] " << #test_func << "\n"; \
    } catch (const std::exception& e) { \
        std::cout << "[ ERROR ] " << #test_func << " threw exception: " << e.what() << "\n"; \
        g_tests_failed++; \
    }

// Helper to extract double result from EngineResult
double GetDouble(const EngineResult& res) {
    if (res.result.has_value() && std::holds_alternative<double>(res.result.value())) {
        return std::get<double>(res.result.value());
    }
    throw std::runtime_error("Result is not a double!");
}

// Helper to extract Vector result
std::vector<double> GetVector(const EngineResult& res) {
    if (res.result.has_value() && std::holds_alternative<Vector>(res.result.value())) {
        return std::get<Vector>(res.result.value());
    }
    throw std::runtime_error("Result is not a Vector!");
}

// Helper to extract Matrix result
Matrix GetMatrix(const EngineResult& res) {
    if (res.result.has_value() && std::holds_alternative<Matrix>(res.result.value())) {
        return std::get<Matrix>(res.result.value());
    }
    throw std::runtime_error("Result is not a Matrix!");
}

// =========================================================================
// TEST CASES
// =========================================================================

void Test_StringHelpers() {
    auto parts = Utils::Split("  a ;  b  ", ';');
    std::cout << "[DEBUG] Split Size: " << parts.size() << std::endl;
    if (parts.size() > 0) std::cout << "[DEBUG] Part 0: '" << parts[0] << "'" << std::endl;
    if (parts.size() > 1) std::cout << "[DEBUG] Part 1: '" << parts[1] << "'" << std::endl;
    // 1. IsNumber Check
    ASSERT_EQ(Utils::IsNumber("123"), true);
    ASSERT_EQ(Utils::IsNumber("-5.43"), true);
    ASSERT_EQ(Utils::IsNumber("abc"), false);
    ASSERT_EQ(Utils::IsNumber("12.34.56"), false);

    // 2. Ans Replacement
    std::string input = "Ans + 5";
    std::string output = Utils::ReplaceAns(input, 10.0);
    // "10 + 5" olmalı (double to string formatına göre 10 veya 10.000)
    // Basit kontrol: Ans yok olmalı
    ASSERT_EQ(output.find("Ans"), std::string::npos);
    ASSERT_EQ(output.substr(0, 2), "10");
}

void Test_AlgebraicBasic() {
    CalcEngine engine;
    engine.SetMode(CalcMode::Algebraic);

    // 1. Order of Operations (PEMDAS)
    // 3 + 5 * 2 = 13 (NOT 16)
    ASSERT_NEAR(GetDouble(engine.Evaluate("3 + 5 * 2")), 13.0, 1e-9);

    // 2. Parentheses
    // (3 + 5) * 2 = 16
    ASSERT_NEAR(GetDouble(engine.Evaluate("(3 + 5) * 2")), 16.0, 1e-9);

    // 3. Negative Numbers
    // -5 + 3 = -2
    ASSERT_NEAR(GetDouble(engine.Evaluate("-5 + 3")), -2.0, 1e-9);
}

void Test_AlgebraicFunctions() {
    CalcEngine engine;
    engine.SetMode(CalcMode::Algebraic);

    // 1. Trigonometry (Degrees)
    // sin(90) = 1
    ASSERT_NEAR(GetDouble(engine.Evaluate("sin 90")), 1.0, 1e-9);
    // cos(180) = -1
    ASSERT_NEAR(GetDouble(engine.Evaluate("cos 180")), -1.0, 1e-9);

    // 2. Sqrt
    ASSERT_NEAR(GetDouble(engine.Evaluate("sqrt 16")), 4.0, 1e-9);

    // 3. Power
    ASSERT_NEAR(GetDouble(engine.Evaluate("2 ^ 3")), 8.0, 1e-9);
}

void Test_NonLinearSolver() {
    CalcEngine engine;
    engine.SetMode(CalcMode::Algebraic);

    // Solve Circle Intersection: x^2 + y^2 = 25, x = 3
    // Expected: y = 4 (or -4, depending on start guess)
    // Input format: solve_nl {eq1; eq2} [guess1, guess2]
    
    // Test Case: Simple Linear within Non-Linear Solver
    // x + y = 10; x - y = 2 -> x=6, y=4
    std::string cmd = "solve_nl {x + y = 10; x - y = 2} [1, 1]";
    
    auto res = GetVector(engine.Evaluate(cmd));
    // Sıralama alfabetik (x, y) olmalı
    ASSERT_NEAR(res[0], 6.0, 1e-3);
    ASSERT_NEAR(res[1], 4.0, 1e-3);
}

void Test_LinearSystemParsing() {
    CalcEngine engine;
    engine.SetMode(CalcMode::LinearSystem);

    // 1. Parse Matrix String
    // Input: [[1, 2], [3, 4]]
    // Expected: Matrix 2x2
    // Bu test için public API üzerinden "qr" gibi bir komut çağırıp hata almadığımızı görebiliriz
    // veya özel bir test parse fonksiyonu yazabiliriz ama motoru test edelim.
    
    // 2x + y = 5; x - y = 1 -> x=2, y=1
    auto res = GetVector(engine.Evaluate("2x + y = 5; x - y = 1"));
    ASSERT_NEAR(res[0], 2.0, 1e-5); // x
    ASSERT_NEAR(res[1], 1.0, 1e-5); // y
}

void Test_MatrixOperations() {
    CalcEngine engine;
    engine.SetMode(CalcMode::LinearSystem);

    // Eigenvalues of Identity Matrix [[1,0],[0,1]] are 1, 1
    // Command: eigen [[1,0],[0,1]]
    auto res = GetVector(engine.Evaluate("eigen [[1, 0], [0, 1]]"));
    ASSERT_NEAR(res[0], 1.0, 1e-5);
    ASSERT_NEAR(res[1], 1.0, 1e-5);
}

int main() {
    std::cout << "======================================\n";
    std::cout << "   OGULATOR BULLETPROOF TEST SUITE    \n";
    std::cout << "======================================\n";

    RUN_TEST(Test_StringHelpers);
    RUN_TEST(Test_AlgebraicBasic);
    RUN_TEST(Test_AlgebraicFunctions);
    RUN_TEST(Test_NonLinearSolver);
    RUN_TEST(Test_LinearSystemParsing);
    RUN_TEST(Test_MatrixOperations);

    std::cout << "======================================\n";
    std::cout << "Tests Passed: " << g_tests_passed << "\n";
    std::cout << "Tests Failed: " << g_tests_failed << "\n";
    
    if (g_tests_failed == 0) {
        std::cout << "ALL SYSTEMS OPERATIONAL. READY FOR LAUNCH.\n";
        return 0;
    } else {
        std::cout << "SYSTEM FAILURE DETECTED.\n";
        return 1;
    }
}