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

enum class CalcMode {
    Algebraic,
    LinearSystem,
    Statistics,
    Symbolic,
    Plotting,
    Units,
    Python,
    PythonNumPy,
    PythonSciPy,
    PythonMatplotlib,
    PythonPandas,
    PythonSymPy
};

class CalcEngine {
private:
    std::map<CalcMode, std::unique_ptr<IParser>> parsers_;
    CalcMode current_mode_ = CalcMode::Algebraic;
    
    // New specialized engines
    std::unique_ptr<UnitManager> unit_manager_;
    std::unique_ptr<SymbolicEngine> symbolic_engine_;
    std::unique_ptr<StatisticsEngine> statistics_engine_;
    std::unique_ptr<PlotEngine> plot_engine_;
#ifdef ENABLE_PYTHON_FFI
    std::unique_ptr<PythonEngine> python_engine_;
#endif

public:
    CalcEngine();

    void SetMode(CalcMode mode);
    EngineResult Evaluate(const std::string& input);
    EngineResult EvaluateWithContext(const std::string& input , const std::map<std::string,double>&context);
    
    // New engine accessors
    UnitManager* GetUnitManager() { return unit_manager_.get(); }
    SymbolicEngine* GetSymbolicEngine() { return symbolic_engine_.get(); }
    StatisticsEngine* GetStatisticsEngine() { return statistics_engine_.get(); }
    PlotEngine* GetPlotEngine() { return plot_engine_.get(); }
#ifdef ENABLE_PYTHON_FFI
    PythonEngine* GetPythonEngine() { return python_engine_.get(); }
#endif
};