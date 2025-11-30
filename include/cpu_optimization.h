/**
 * @file cpu_optimization.h
 * @brief CPU-specific optimizations and performance tuning
 */

#ifndef CPU_OPTIMIZATION_H
#define CPU_OPTIMIZATION_H

#include <string>
#include <vector>
#include <memory>

namespace AXIOM {

/**
 * @brief CPU optimization utilities
 */
class CPUOptimization {
public:
    static void Initialize();
    static std::string GetCPUInfo();
    static void OptimizeForCurrentCPU();
    
private:
    static bool DetectSSE();
    static bool DetectAVX();
    static bool DetectAVX2();
};

} // namespace AXIOM

#endif // CPU_OPTIMIZATION_H