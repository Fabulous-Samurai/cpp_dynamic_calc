/**
 * @file eigen_engine.h
 * @brief AXIOM v3.1 - Advanced Mathematical Functions Engine
 * 
 * High-performance engine with complex number support, SIMD acceleration,
 * advanced mathematical functions, and optimized linear algebra operations.
 */

#pragma once

#include "../../include/dynamic_calc_types.h"
#include <memory>
#include <string>
#include <vector>
#include <chrono>

#ifdef ENABLE_EIGEN
#include <Eigen/Core>
#endif

namespace AXIOM {

/**
 * @brief CPU optimization levels for performance tuning
 */
enum class CPUOptimizationLevel {
    Basic,
    SIMD,
    Parallel,
    Vectorized,
    Extreme
};

/**
 * @brief Performance metrics for CPU operations
 */
struct CPUPerformanceMetrics {
    size_t total_operations = 0;
    size_t simd_operations = 0;
    size_t parallel_operations = 0;
    size_t complex_operations = 0;
    size_t fast_path_operations = 0;
    size_t fft_operations = 0;
    size_t cache_hits = 0;
    size_t cache_misses = 0;
    long long last_execution_time_us = 0;
    double avg_throughput_mops = 0.0;
    double senna_speed_ratio = 1.0;
};

/**
 * @brief High-performance mathematical engine with complex number support
 */
class EigenEngine {
public:
    EigenEngine();
    ~EigenEngine();

    void SetOptimizationLevel(CPUOptimizationLevel level);
    void EnableSIMD(bool enable = true);
    void SetNumThreads(int num_threads = -1);

    EngineResult Calculate(const std::string& expression);
    EngineResult CalculateComplex(const std::string& expression);
    
    EngineResult CalculateFourier(const std::vector<AXIOM::Number>& signal);
    EngineResult CalculateConvolution(const std::vector<AXIOM::Number>& signal1, const std::vector<AXIOM::Number>& signal2);
    EngineResult CalculatePolynomialRoots(const std::vector<AXIOM::Number>& coefficients);
    EngineResult CalculateEigenvalues(const std::vector<std::vector<AXIOM::Number>>& matrix);
    
    AXIOM::Number DotProductComplex(const std::vector<AXIOM::Number>& v1, const std::vector<AXIOM::Number>& v2);
    AXIOM::Number DeterminantComplex(const std::vector<std::vector<AXIOM::Number>>& matrix);
    double DotProduct(const std::vector<double>& v1, const std::vector<double>& v2);
    double Determinant(const std::vector<std::vector<double>>& matrix);
    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> QRDecomposition(const std::vector<std::vector<double>>& matrix);
    std::vector<std::vector<double>> MatrixMultiply(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B);
    
    std::vector<std::vector<AXIOM::Number>> MatrixMultiplyComplex(const std::vector<std::vector<AXIOM::Number>>& A, const std::vector<std::vector<AXIOM::Number>>& B);
    std::pair<std::vector<std::vector<AXIOM::Number>>, std::vector<std::vector<AXIOM::Number>>> QRDecompositionComplex(const std::vector<std::vector<AXIOM::Number>>& matrix);

    bool IsComplexOperation(const std::string& expression) const;
    bool IsAdvancedMathOperation(const std::string& expression) const;
    bool IsSignalProcessingOperation(const std::string& expression) const;
    
    EngineResult HandleComplexOperation(const std::string& expression);
    EngineResult HandleAdvancedMathOperation(const std::string& expression);
    EngineResult HandleSignalProcessingOperation(const std::string& expression);

    CPUPerformanceMetrics GetLastMetrics() const;
    std::string GetPerformanceReport() const;

private:
#ifdef ENABLE_EIGEN
    EngineResult DispatchToEigenOperation(const std::string& expression);
    
    bool IsVectorOperation(const std::string& expression) const;
    bool IsMatrixOperation(const std::string& expression) const;
    bool IsLinearAlgebraOperation(const std::string& expression) const;
    
    EngineResult HandleScalarOperation(const std::string& expression);
    EngineResult HandleVectorOperation(const std::string& expression);
    EngineResult HandleMatrixOperation(const std::string& expression);
    EngineResult HandleLinearAlgebraOperation(const std::string& expression);
    
    double ExtractNumericValue(const std::string& expression) const;
    AXIOM::Number ExtractComplexValue(const std::string& expression) const;
    std::vector<double> ParseVector(const std::string& expression) const;
    std::vector<AXIOM::Number> ParseComplexVector(const std::string& expression) const;
    std::vector<std::vector<double>> ParseMatrix(const std::string& expression) const;
    std::vector<std::vector<AXIOM::Number>> ParseComplexMatrix(const std::string& expression) const;
    
#ifdef ENABLE_EIGEN
    std::vector<std::vector<double>> EigenMatrixToStdMatrix(const Eigen::MatrixXd& eigen_matrix) const;
    std::vector<std::vector<AXIOM::Number>> EigenComplexMatrixToStdMatrix(const Eigen::MatrixXcd& eigen_matrix) const;
    Eigen::MatrixXcd StdComplexMatrixToEigen(const std::vector<std::vector<AXIOM::Number>>& std_matrix) const;
#endif
#endif

    CPUOptimizationLevel optimization_level_;
    bool simd_enabled_;
    int num_threads_;
    mutable CPUPerformanceMetrics last_metrics_;
};

/**
 * @brief RAII Performance Timer for automatic measurement
 */
class PerformanceTimer {
public:
    explicit PerformanceTimer(const std::string& operation_name);
    ~PerformanceTimer();

private:
    std::string operation_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

#define MEASURE_PERFORMANCE(op_name) PerformanceTimer timer(op_name)
#define SENNA_SPEED_EIGEN(op_name) PerformanceTimer timer("🏎️ " + std::string(op_name))
#define AXIOM_COMPLEX_PERFORMANCE(op_name) PerformanceTimer timer("🔢 " + std::string(op_name))
#define AXIOM_FFT_PERFORMANCE(op_name) PerformanceTimer timer("📡 " + std::string(op_name))
#define AXIOM_ADVANCED_MATH(op_name) PerformanceTimer timer("🧮 " + std::string(op_name))

namespace MathConstants {
    constexpr double EULER_NUMBER = 2.71828182845904523536;
    constexpr double GOLDEN_RATIO = 1.61803398874989484820;
    constexpr double SQRT_2 = 1.41421356237309504880;
    constexpr double SQRT_PI = 1.77245385090551602729;
    constexpr double LN_2 = 0.69314718055994530941;
    constexpr double LN_10 = 2.30258509299404568401;
    
    const AXIOM::Number IMAGINARY_UNIT(std::complex<double>(0.0, 1.0));
    const AXIOM::Number EULER_COMPLEX(std::complex<double>(EULER_NUMBER, 0.0));
}

} // namespace AXIOM