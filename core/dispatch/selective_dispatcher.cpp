/**
 * @file selective_dispatcher.cpp
 * @brief AXIOM v3.0 - Intelligent Selective Dispatcher Implementation
 * 
 * Advanced operation routing system that selects optimal computational engines
 * based on expression complexity, data size, and performance characteristics.
 */

#include "selective_dispatcher.h"
#include "../../include/dynamic_calc.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace AXIOM {

SelectiveDispatcher::SelectiveDispatcher() 
    : preferred_engine_(ComputeEngine::Auto)
    , fallback_enabled_(true)
    , performance_threshold_ms_(100.0)
    , learning_enabled_(true) {
    
    // Engine instances temporarily disabled until classes are fully implemented
    engine_availability_[ComputeEngine::Native] = true;
    engine_availability_[ComputeEngine::Eigen] = false;  // Enable when Eigen is available
    engine_availability_[ComputeEngine::Python] = false; // Enable when nanobind is available
    
// Eigen engine temporarily disabled until EigenEngine class is implemented
// #ifdef ENABLE_EIGEN
//     try {
//         eigen_engine_ = std::make_unique<EigenEngine>();
//         engine_availability_[ComputeEngine::Eigen] = true;
//         std::cout << "✅ Eigen CPU Engine initialized\n";
//     } catch (const std::exception& e) {
//         std::cerr << "⚠️ Eigen Engine initialization failed: " << e.what() << "\n";
//     }
// #endif

// Python engine temporarily disabled until PythonEngine class is implemented
    // #ifdef ENABLE_NANOBIND
    // try {
    //     python_engine_ = std::make_unique<PythonEngine>();
    //     engine_availability_[ComputeEngine::Python] = true;
    //     std::cout << "✅ Python Engine (nanobind) initialized\n";
    // } catch (const std::exception& e) {
    //     std::cerr << "⚠️ Python Engine initialization failed: " << e.what() << "\n";
    // }
    // #endif
}

SelectiveDispatcher::~SelectiveDispatcher() = default;

EngineResult SelectiveDispatcher::DispatchOperation(const std::string& expression,
                                                   OperationComplexity complexity) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 1. Analyze expression and determine optimal engine
    ComputeEngine selected_engine = SelectOptimalEngine(expression, complexity);
    
    // 2. Execute on selected engine with fallback
    EngineResult result = ExecuteWithFallback(expression, selected_engine);
    
    // 3. Record performance metrics
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    RecordMetrics(selected_engine, expression, complexity, duration.count() / 1000.0);
    
    return result;
}

ComputeEngine SelectiveDispatcher::SelectOptimalEngine(const std::string& expression, 
                                                      OperationComplexity complexity) {
    // For initial implementation, use Native engine only
    return ComputeEngine::Native;
    
    // Future implementation will analyze expression characteristics:
    /*
    // Override for preferred engine if specified
    if (preferred_engine_ != ComputeEngine::Auto && 
        IsEngineAvailable(preferred_engine_)) {
        return preferred_engine_;
    }
    
    // Analyze expression characteristics
    size_t data_size = EstimateDataSize(expression);
    bool has_matrix_ops = HasMatrixOperations(expression);
    bool has_symbolic_ops = HasSymbolicOperations(expression);
    
    // Engine selection logic based on operation type and complexity
    if (has_symbolic_ops && IsEngineAvailable(ComputeEngine::Python)) {
        return ComputeEngine::Python;  // Python for symbolic math
    }
    
    if (has_matrix_ops && data_size > 1000 && IsEngineAvailable(ComputeEngine::Eigen)) {
        return ComputeEngine::Eigen;   // Eigen for large matrix operations
    }
    
    if (complexity >= OperationComplexity::Complex && IsEngineAvailable(ComputeEngine::Eigen)) {
        return ComputeEngine::Eigen;   // Eigen for complex numerical computations
    }
    */
    
    // Default to native engine for simple operations
    return ComputeEngine::Native;
}

EngineResult SelectiveDispatcher::ExecuteWithFallback(const std::string& expression,
                                                     ComputeEngine engine) {
    EngineResult result;
    
    // For initial implementation, only use native engine
    result = ExecuteNative(expression);
    return result;
    
    /*
    try {
        switch (engine) {
            case ComputeEngine::Native:
                result = ExecuteNative(expression);
                break;
                
            case ComputeEngine::Eigen:
#ifdef ENABLE_EIGEN
                if (eigen_engine_) {
                    result = eigen_engine_->Calculate(expression);
                    break;
                }
#endif
                // Fallback to native
                result = ExecuteNative(expression);
                break;
                
            case ComputeEngine::Python:
                // Python engine temporarily disabled until PythonEngine class is implemented
                // #ifdef ENABLE_NANOBIND
                // if (python_engine_) {
                //     result = python_engine_->Calculate(expression);
                //     break;
                // }
                // #endif
                // Fallback to native
                result = ExecuteNative(expression);
                break;
                
            default:
                result = ExecuteNative(expression);
                break;
        }
    } catch (const std::exception& e) {
        // Fallback on any engine failure
        if (fallback_enabled_ && engine != ComputeEngine::Native) {
            std::cerr << "Engine " << EngineToString(engine) << " failed, falling back to Native\n";
            result = ExecuteNative(expression);
        } else {
            result = {{}, {CalcErr::OperationNotFound}};
        }
    }
    */
}

EngineResult SelectiveDispatcher::ExecuteNative(const std::string& expression) {
    // Use the existing AXIOM native engine
    DynamicCalc calc;
    return calc.calculate(expression, CalculationMode::ALGEBRAIC);
}

bool SelectiveDispatcher::IsEngineAvailable(ComputeEngine engine) const {
    auto it = engine_availability_.find(engine);
    return (it != engine_availability_.end()) && it->second;
}

size_t SelectiveDispatcher::EstimateDataSize(const std::string& expression) const {
    // Simple heuristic for data size estimation
    size_t base_size = expression.length();
    
    // Count matrix/vector indicators
    size_t matrix_count = std::count(expression.begin(), expression.end(), '[');
    size_t comma_count = std::count(expression.begin(), expression.end(), ',');
    
    return base_size + (matrix_count * 100) + (comma_count * 10);
}

bool SelectiveDispatcher::HasMatrixOperations(const std::string& expression) const {
    return (expression.find('[') != std::string::npos) ||
           (expression.find("matrix") != std::string::npos) ||
           (expression.find("solve") != std::string::npos) ||
           (expression.find("linear") != std::string::npos);
}

bool SelectiveDispatcher::HasSymbolicOperations(const std::string& expression) const {
    return (expression.find("symbolic") != std::string::npos) ||
           (expression.find("derivative") != std::string::npos) ||
           (expression.find("integrate") != std::string::npos) ||
           (expression.find("expand") != std::string::npos) ||
           (expression.find("factor") != std::string::npos);
}

void SelectiveDispatcher::RecordMetrics(ComputeEngine engine, 
                                       const std::string& expression,
                                       OperationComplexity complexity,
                                       double execution_time_ms) {
    // Store performance data for learning
    last_metrics_.selected_engine = engine;
    last_metrics_.operation_name = expression.substr(0, 20); // First 20 chars
    last_metrics_.complexity = complexity;
    last_metrics_.execution_time_ms = execution_time_ms;
    last_metrics_.decision_time_us = 0.1; // Minimal overhead
    
    // Update engine performance history
    auto& perf = engine_performance_[engine][expression.substr(0, 10)];
    perf.operations_count++;
    perf.avg_execution_time_ms = (perf.avg_execution_time_ms * (perf.operations_count - 1) + 
                                 execution_time_ms) / perf.operations_count;
    perf.engine_type = engine;
}

std::string SelectiveDispatcher::EngineToString(ComputeEngine engine) const {
    switch (engine) {
        case ComputeEngine::Native: return "Native C++";
        case ComputeEngine::Eigen: return "Eigen CPU";
        case ComputeEngine::Python: return "Python/nanobind";
        case ComputeEngine::Auto: return "Auto-Select";
        default: return "Unknown";
    }
}

DispatchMetrics SelectiveDispatcher::GetLastMetrics() const {
    return last_metrics_;
}

std::string SelectiveDispatcher::GetPerformanceReport() const {
    std::ostringstream report;
    report << "🎯 AXIOM v3.0 - Selective Dispatcher Performance Report\n";
    report << "=====================================================\n\n";
    
    report << "🔧 Engine Availability:\n";
    for (const auto& [engine, available] : engine_availability_) {
        report << "  " << (available ? "✅" : "❌") << " " 
               << EngineToString(engine) << "\n";
    }
    
    report << "\n📊 Performance Metrics:\n";
    for (const auto& [engine, ops] : engine_performance_) {
        if (ops.empty()) continue;
        
        report << "  " << EngineToString(engine) << ":\n";
        size_t total_ops = 0;
        double avg_time = 0.0;
        
        for (const auto& [op, perf] : ops) {
            total_ops += perf.operations_count;
            avg_time += perf.avg_execution_time_ms * perf.operations_count;
        }
        
        if (total_ops > 0) {
            avg_time /= total_ops;
            report << "    Operations: " << total_ops << "\n";
            report << "    Avg Time: " << avg_time << "ms\n";
            
            // Performance classification
            if (avg_time < 1.0) {
                report << "    Grade: 🏎️ SENNA SPEED\n";
            } else if (avg_time < 10.0) {
                report << "    Grade: 🏁 F1 SPEED\n";
            } else if (avg_time < 100.0) {
                report << "    Grade: 🚗 GOOD SPEED\n";
            } else {
                report << "    Grade: 🐌 NEEDS OPTIMIZATION\n";
            }
        }
        report << "\n";
    }
    
    report << "📈 Last Operation:\n";
    report << "  Engine: " << EngineToString(last_metrics_.selected_engine) << "\n";
    report << "  Time: " << last_metrics_.execution_time_ms << "ms\n";
    report << "  Complexity: " << static_cast<int>(last_metrics_.complexity) << "\n";
    
    return report.str();
}

void SelectiveDispatcher::SetPreferredEngine(ComputeEngine engine) {
    preferred_engine_ = engine;
}

void SelectiveDispatcher::EnableLearning(bool enable) {
    learning_enabled_ = enable;
}

void SelectiveDispatcher::SetPerformanceThreshold(double threshold_ms) {
    performance_threshold_ms_ = threshold_ms;
}

} // namespace AXIOM