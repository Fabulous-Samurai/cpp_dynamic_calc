#pragma once

#include "IParser.h"
#include "dynamic_calc_types.h"
#include "unit_manager.h"
#include "symbolic_engine.h"
#include "statistics_engine.h"
#include "plot_engine.h"
#ifdef ENABLE_PYTHON_FFI
#include "python_parser.h"
#include "python_engine.h"
#endif
#include <map>
#include <memory>
#include <string>

namespace AXIOM {

enum class CalculationMode {
    ALGEBRAIC,
    LINEAR_SYSTEM,
    STATISTICS,
    SYMBOLIC,
    UNITS,
    PLOT,
    PYTHON,
    PYTHON_NUMPY,
    PYTHON_SCIPY,
    PYTHON_MATPLOTLIB,
    PYTHON_PANDAS,
    PYTHON_SYMPY
};

// Legacy alias for backwards compatibility
using CalcMode = CalculationMode;

// Helper function to convert mode to string
std::string mode_to_string(CalculationMode mode);

class DynamicCalc {
private:
    std::map<CalculationMode, std::unique_ptr<IParser>> parsers_;
    CalculationMode current_mode_ = CalculationMode::ALGEBRAIC;
    
    // New specialized engines
    std::unique_ptr<UnitManager> unit_manager_;
    std::unique_ptr<SymbolicEngine> symbolic_engine_;
    std::unique_ptr<StatisticsEngine> statistics_engine_;
    std::unique_ptr<PlotEngine> plot_engine_;
// Python engine temporarily disabled until class is fully implemented
// #ifdef ENABLE_PYTHON_FFI
//     std::unique_ptr<PythonEngine> python_engine_;
// #endif

public:
    DynamicCalc();

    void SetMode(CalculationMode mode);
    EngineResult Evaluate(const std::string& input);
    EngineResult calculate(const std::string& input, CalculationMode mode = CalculationMode::ALGEBRAIC);
    EngineResult EvaluateWithContext(const std::string& input , const std::map<std::string,double>&context);
    
    // New engine accessors
    UnitManager* GetUnitManager() { return unit_manager_.get(); }
    SymbolicEngine* GetSymbolicEngine() { return symbolic_engine_.get(); }
    StatisticsEngine* GetStatisticsEngine() { return statistics_engine_.get(); }
    PlotEngine* GetPlotEngine() { return plot_engine_.get(); }
#ifdef ENABLE_PYTHON_FFI
    PythonEngine* GetPythonEngine() { return nullptr; } // Disabled for pure C++ performance
#endif
};

} // namespace AXIOM