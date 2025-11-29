/**
 * @file main.cpp
 * @brief Entry point for the Ogulator TUI application.
 * Updated to support Context-based 'Ans' variable.
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>
#include <iomanip>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

#include "dynamic_calc.h"
#include "string_helpers.h"

#ifdef ENABLE_EIGEN
#include "eigen_engine.h"
#endif

// Temporarily disabled until nanobind headers are resolved
//#ifdef ENABLE_NANOBIND
//#include "nanobind_interface.h"
//#endif

#include "selective_dispatcher.h"
#include "cpu_optimization.h"

#ifdef ENABLE_PYTHON_FFI
#include "python_repl.h"
#endif
#include "version.h"

using namespace ftxui;

struct HistoryLine {
    std::string content;
    Color color;
    bool is_bold;
};

void PrintAlgebraicHelp(std::vector<HistoryLine>& history) {
    history.push_back({"=== ALGEBRAIC MODE HELP ===", Color::Cyan, true});
    history.push_back({"Advanced scientific calculator with calculus functions", Color::GrayLight, false});
    history.push_back({"", Color::White, false});
    
    history.push_back({"[BASIC OPERATIONS]", Color::Yellow, true});
    history.push_back({"  +, -, *, /     : Basic arithmetic", Color::White, false});
    history.push_back({"  ^              : Exponentiation (2^3 = 8)", Color::White, false});
    history.push_back({"  %              : Modulo (10 % 3 = 1)", Color::White, false});
    history.push_back({"  ()             : Parentheses for grouping", Color::White, false});
    
    history.push_back({"[FUNCTIONS]", Color::Yellow, true});
    history.push_back({"  sin(x), cos(x), tan(x) : Trigonometric (degrees)", Color::White, false});
    history.push_back({"  asin(x), acos(x), atan(x) : Inverse trig", Color::White, false});
    history.push_back({"  log(x)         : Base-10 logarithm", Color::White, false});
    history.push_back({"  ln(x)          : Natural logarithm", Color::White, false});
    history.push_back({"  exp(x)         : Exponential (e^x)", Color::White, false});
    history.push_back({"  sqrt(x)        : Square root", Color::White, false});
    history.push_back({"  abs(x)         : Absolute value", Color::White, false});
    history.push_back({"  factorial(x)   : Factorial function", Color::White, false});
    
    history.push_back({"[CALCULUS FUNCTIONS - NEW!]", Color::Yellow, true});
    history.push_back({"  limit(expr, var, point) : Numerical limit", Color::Green, false});
    history.push_back({"    Example: limit(sin(x)/x, x, 0) = 1", Color::GrayLight, false});
    history.push_back({"  integrate(expr, var, a, b) : Definite integral", Color::Green, false});
    history.push_back({"    Example: integrate(x^2, x, 0, 2) = 2.67", Color::GrayLight, false});
    
    history.push_back({"[CONSTANTS & VARIABLES]", Color::Yellow, true});
    history.push_back({"  pi             : π (3.14159...)", Color::White, false});
    history.push_back({"  e              : Euler's number (2.71828...)", Color::White, false});
    history.push_back({"  Ans            : Previous result", Color::White, false});
    history.push_back({"  x, y, z        : Variables in expressions", Color::White, false});
    
    history.push_back({"[EXAMPLES]", Color::Yellow, true});
    history.push_back({"  2 + 3 * 4      → 14", Color::Cyan, false});
    history.push_back({"  sin(45)        → 0.707", Color::Cyan, false});
    history.push_back({"  sqrt(Ans)      → Uses previous result", Color::Cyan, false});
    history.push_back({"  limit((x^2-1)/(x-1), x, 1) → 2", Color::Green, false});
    history.push_back({"  integrate(x, x, 0, 2) → 2", Color::Green, false});
    history.push_back({"", Color::White, false});
    history.push_back({"Type 'help modes' for other calculator modes", Color::GrayLight, false});
}

void PrintLinearHelp(std::vector<HistoryLine>& history) {
    history.push_back({"=== LINEAR SYSTEM MODE HELP ===", Color::Cyan, true});
    history.push_back({"Matrix operations and system solving", Color::GrayLight, false});
    history.push_back({"", Color::White, false});
    
    history.push_back({"[MATRIX INPUT FORMAT]", Color::Yellow, true});
    history.push_back({"  [1,2;3,4]      : 2x2 matrix", Color::White, false});
    history.push_back({"  [a,b,c;d,e,f]  : 2x3 matrix", Color::White, false});
    history.push_back({"  Use ';' to separate rows", Color::GrayLight, false});
    history.push_back({"  Use ',' to separate columns", Color::GrayLight, false});
    
    history.push_back({"[MATRIX OPERATIONS]", Color::Yellow, true});
    history.push_back({"  A + B          : Matrix addition", Color::White, false});
    history.push_back({"  A - B          : Matrix subtraction", Color::White, false});
    history.push_back({"  A * B          : Matrix multiplication", Color::White, false});
    history.push_back({"  det(A)         : Determinant", Color::White, false});
    history.push_back({"  inv(A)         : Matrix inverse", Color::White, false});
    history.push_back({"  transpose(A)   : Matrix transpose", Color::White, false});
    
    history.push_back({"[SYSTEM SOLVING]", Color::Yellow, true});
    history.push_back({"  solve([2,1;1,3], [5;7]) : Solve Ax=b", Color::White, false});
    history.push_back({"  Will find x such that Ax = b", Color::GrayLight, false});
    
    history.push_back({"[EXAMPLES]", Color::Yellow, true});
    history.push_back({"  [1,2;3,4] * [5,6;7,8] → Matrix product", Color::Cyan, false});
    history.push_back({"  det([1,2;3,4]) → -2", Color::Cyan, false});
    history.push_back({"  solve([2,1;1,1], [3;2]) → [1, 1]", Color::Cyan, false});
    history.push_back({"  System: 2x+y=3, x+y=2 → x=1, y=1", Color::GrayLight, false});
}

void PrintStatsHelp(std::vector<HistoryLine>& history) {
    history.push_back({"=== STATISTICS MODE HELP ===", Color::Cyan, true});
    history.push_back({"Statistical analysis and data operations", Color::GrayLight, false});
    history.push_back({"", Color::White, false});
    
    history.push_back({"[DATA INPUT]", Color::Yellow, true});
    history.push_back({"  [1,2,3,4,5]    : Data vector", Color::White, false});
    history.push_back({"  Use commas to separate values", Color::GrayLight, false});
    
    history.push_back({"[DESCRIPTIVE STATISTICS]", Color::Yellow, true});
    history.push_back({"  mean([1,2,3,4,5]) : Average value", Color::White, false});
    history.push_back({"  median([1,2,3,4,5]) : Middle value", Color::White, false});
    history.push_back({"  stddev([1,2,3,4,5]) : Standard deviation", Color::White, false});
    history.push_back({"  variance([1,2,3,4,5]) : Variance", Color::White, false});
    history.push_back({"  min([1,2,3,4,5]) : Minimum value", Color::White, false});
    history.push_back({"  max([1,2,3,4,5]) : Maximum value", Color::White, false});
    history.push_back({"  sum([1,2,3,4,5]) : Total sum", Color::White, false});
    
    history.push_back({"[DISTRIBUTION FUNCTIONS]", Color::Yellow, true});
    history.push_back({"  normal(x, mu, sigma) : Normal PDF", Color::White, false});
    history.push_back({"  binomial(k, n, p) : Binomial probability", Color::White, false});
    history.push_back({"  poisson(k, lambda) : Poisson probability", Color::White, false});
    
    history.push_back({"[EXAMPLES]", Color::Yellow, true});
    history.push_back({"  mean([10,20,30,40,50]) → 30", Color::Cyan, false});
    history.push_back({"  stddev([1,2,3,4,5]) → 1.58", Color::Cyan, false});
    history.push_back({"  normal(0, 0, 1) → 0.399 (std normal at x=0)", Color::Cyan, false});
}

void PrintPlotHelp(std::vector<HistoryLine>& history) {
    history.push_back({"=== PLOTTING MODE HELP ===", Color::Cyan, true});
    history.push_back({"ASCII function plotting and visualization", Color::GrayLight, false});
    history.push_back({"", Color::White, false});
    
    history.push_back({"[PLOT COMMAND FORMAT]", Color::Yellow, true});
    history.push_back({"  plot(function, xmin, xmax, ymin, ymax)", Color::White, false});
    history.push_back({"  All parameters are required", Color::GrayLight, false});
    
    history.push_back({"[SUPPORTED FUNCTIONS]", Color::Yellow, true});
    history.push_back({"  Linear: x, 2*x+1, -x+5", Color::White, false});
    history.push_back({"  Polynomial: x^2, x^3-2*x, x^4-4*x^2", Color::White, false});
    history.push_back({"  Trigonometric: sin(x), cos(x), tan(x)", Color::White, false});
    history.push_back({"  Exponential: exp(x), log(x), ln(x)", Color::White, false});
    history.push_back({"  Other: abs(x), sqrt(x), 1/x", Color::White, false});
    
    history.push_back({"[PLOT EXAMPLES]", Color::Yellow, true});
    history.push_back({"  plot(x^2, -3, 3, 0, 9) : Parabola", Color::Cyan, false});
    history.push_back({"  plot(sin(x), 0, 360, -1.5, 1.5) : Sine wave", Color::Cyan, false});
    history.push_back({"  plot(exp(x), -2, 2, 0, 8) : Exponential", Color::Cyan, false});
    history.push_back({"  plot(abs(x), -5, 5, 0, 5) : V-shape", Color::Cyan, false});
    
    history.push_back({"[TIPS]", Color::Yellow, true});
    history.push_back({"  • Choose appropriate x/y ranges for best view", Color::White, false});
    history.push_back({"  • Functions work in any mode with 'plot' prefix", Color::White, false});
    history.push_back({"  • ASCII plots show general function shape", Color::White, false});
    history.push_back({"  • See tests/PLOTTING_TEST_CASES.md for more examples", Color::Green, false});
}

void PrintUnitsHelp(std::vector<HistoryLine>& history) {
    history.push_back({"=== UNITS MODE HELP ===", Color::Cyan, true});
    history.push_back({"Unit conversions and dimensional analysis", Color::GrayLight, false});
    history.push_back({"", Color::White, false});
    
    history.push_back({"[CONVERSION FORMAT]", Color::Yellow, true});
    history.push_back({"  convert VALUE UNIT to UNIT", Color::White, false});
    history.push_back({"  Example: convert 5 m to ft", Color::GrayLight, false});
    
    history.push_back({"[LENGTH UNITS]", Color::Yellow, true});
    history.push_back({"  m, cm, mm, km, ft, in, yd, mile", Color::White, false});
    
    history.push_back({"[MASS UNITS]", Color::Yellow, true});
    history.push_back({"  kg, g, mg, lb, oz, ton", Color::White, false});
    
    history.push_back({"[TIME UNITS]", Color::Yellow, true});
    history.push_back({"  s, ms, min, hr, day, week, year", Color::White, false});
    
    history.push_back({"[TEMPERATURE UNITS]", Color::Yellow, true});
    history.push_back({"  C, F, K (Celsius, Fahrenheit, Kelvin)", Color::White, false});
    
    history.push_back({"[AREA & VOLUME]", Color::Yellow, true});
    history.push_back({"  m2, cm2, ft2, in2 (area)", Color::White, false});
    history.push_back({"  m3, cm3, L, mL, gal, qt (volume)", Color::White, false});
    
    history.push_back({"[EXAMPLES]", Color::Yellow, true});
    history.push_back({"  convert 100 cm to m → 1 m", Color::Cyan, false});
    history.push_back({"  convert 32 F to C → 0°C", Color::Cyan, false});
    history.push_back({"  convert 1 mile to km → 1.609 km", Color::Cyan, false});
    history.push_back({"  convert 2.5 kg to lb → 5.51 lb", Color::Cyan, false});
}

void PrintSymbolicHelp(std::vector<HistoryLine>& history) {
    history.push_back({"=== SYMBOLIC MODE HELP ===", Color::Cyan, true});
    history.push_back({"Symbolic mathematics and expression manipulation", Color::GrayLight, false});
    history.push_back({"", Color::White, false});
    
    history.push_back({"[SYMBOLIC OPERATIONS]", Color::Yellow, true});
    history.push_back({"  derive(expr, var) : Symbolic derivative", Color::White, false});
    history.push_back({"  expand(expr)      : Expand algebraic expressions", Color::White, false});
    history.push_back({"  factor(expr)      : Factor expressions", Color::White, false});
    history.push_back({"  simplify(expr)    : Simplify expressions", Color::White, false});
    
    history.push_back({"[DERIVATIVE EXAMPLES]", Color::Yellow, true});
    history.push_back({"  derive(x^2, x) → 2*x", Color::Cyan, false});
    history.push_back({"  derive(sin(x), x) → cos(x)", Color::Cyan, false});
    history.push_back({"  derive(x^3 + 2*x, x) → 3*x^2 + 2", Color::Cyan, false});
    
    history.push_back({"[ALGEBRAIC MANIPULATION]", Color::Yellow, true});
    history.push_back({"  expand((x+1)^2) → x^2 + 2*x + 1", Color::Cyan, false});
    history.push_back({"  factor(x^2 - 4) → (x-2)*(x+2)", Color::Cyan, false});
    history.push_back({"  simplify(2*x + 3*x) → 5*x", Color::Cyan, false});
    
    history.push_back({"[STATUS]", Color::Yellow, true});
    history.push_back({"  Currently in development", Color::Yellow, false});
    history.push_back({"  Basic derivative functionality available", Color::White, false});
    history.push_back({"  More features coming soon!", Color::White, false});
}

void PrintGeneralHelp(std::vector<HistoryLine>& history) {
    history.push_back({"=== OGULATOR HELP SYSTEM ===", Color::Cyan, true});
    history.push_back({"Multi-mode scientific calculator", Color::GrayLight, false});
    history.push_back({"", Color::White, false});
    
    history.push_back({"[NAVIGATION]", Color::Yellow, true});
    history.push_back({"  UP/DOWN Arrow : Cycle command history", Color::White, false});
    history.push_back({"  PgUp/PgDn     : Scroll output log", Color::White, false});
    history.push_back({"  Enter         : Execute command", Color::White, false});
    
    history.push_back({"[GLOBAL COMMANDS]", Color::Yellow, true});
    history.push_back({"  help           : Show this help", Color::White, false});
    history.push_back({"  help [mode]    : Mode-specific help", Color::White, false});
    history.push_back({"  clear          : Clear screen", Color::White, false});
    history.push_back({"  exit           : Close application", Color::White, false});
    
    history.push_back({"[CALCULATOR MODES]", Color::Yellow, true});
    history.push_back({"  mode algebraic : Scientific calculator", Color::Green, false});
    history.push_back({"    → Advanced math, calculus functions", Color::GrayLight, false});
    history.push_back({"  mode linear    : Matrix/system solver", Color::Green, false});
    history.push_back({"    → Linear algebra, matrix operations", Color::GrayLight, false});
    history.push_back({"  mode stats     : Statistical analysis", Color::Green, false});
    history.push_back({"    → Descriptive stats, distributions", Color::GrayLight, false});
    history.push_back({"  mode plot      : Function plotting", Color::Green, false});
    history.push_back({"    → ASCII graphs, function visualization", Color::GrayLight, false});
    history.push_back({"  mode units     : Unit conversions", Color::Green, false});
    history.push_back({"    → Length, mass, temperature, etc.", Color::GrayLight, false});
    history.push_back({"  mode symbolic  : Symbolic math [BETA]", Color::Green, false});
    history.push_back({"    → Derivatives, algebraic manipulation", Color::GrayLight, false});
    
#ifdef ENABLE_PYTHON_FFI
    history.push_back({"[PYTHON INTEGRATION]", Color::Yellow, true});
    history.push_back({"  mode python    : Interactive Python", Color::Green, false});
    history.push_back({"    → Full Python interpreter access", Color::GrayLight, false});
    history.push_back({"  python / py    : Enter Python REPL mode", Color::Green, false});
    history.push_back({"    → Interactive Python shell within calculator", Color::GrayLight, false});
    history.push_back({"  mode numpy     : NumPy scientific computing", Color::Green, false});
    history.push_back({"    → Arrays, linear algebra, FFT", Color::GrayLight, false});
    history.push_back({"  mode scipy     : SciPy advanced math", Color::Green, false});
    history.push_back({"    → Optimization, integration, signal processing", Color::GrayLight, false});
    history.push_back({"  mode matplotlib: Plotting & visualization", Color::Green, false});
    history.push_back({"    → Advanced plotting with Python", Color::GrayLight, false});
    history.push_back({"  mode pandas    : Data analysis", Color::Green, false});
    history.push_back({"    → DataFrames, data manipulation", Color::GrayLight, false});
    history.push_back({"  mode sympy     : Symbolic mathematics", Color::Green, false});
    history.push_back({"    → Advanced symbolic computation", Color::GrayLight, false});
#endif
    
    history.push_back({"[UNIVERSAL COMMANDS]", Color::Yellow, true});
    history.push_back({"  plot(func, x1, x2, y1, y2) : Plot in any mode", Color::White, false});
    history.push_back({"  convert X unit to unit     : Convert in any mode", Color::White, false});
    history.push_back({"  derive(expr, var)          : Derive in any mode", Color::White, false});
    
    history.push_back({"[DETAILED HELP]", Color::Yellow, true});
    history.push_back({"  help algebraic : Calculus & scientific functions", Color::Cyan, false});
    history.push_back({"  help linear    : Matrix operations guide", Color::Cyan, false});
    history.push_back({"  help stats     : Statistical functions", Color::Cyan, false});
    history.push_back({"  help plot      : Function plotting guide", Color::Cyan, false});
    history.push_back({"  help units     : Unit conversion reference", Color::Cyan, false});
    history.push_back({"  help symbolic  : Symbolic math operations", Color::Cyan, false});
    
    history.push_back({"", Color::White, false});
    history.push_back({"🚀 NEW: Advanced calculus functions in algebraic mode!", Color::Green, true});
    history.push_back({"Try: limit(sin(x)/x, x, 0) or integrate(x^2, x, 0, 2)", Color::Green, false});
}

void PrintHelp(std::vector<HistoryLine>& history, const std::string& target) {
    if (target.empty() || target == "general" || target == "help") {
        PrintGeneralHelp(history);
    }
    else if (target == "algebraic") {
        PrintAlgebraicHelp(history);
    }
    else if (target == "linear") {
        PrintLinearHelp(history);
    }
    else if (target == "stats" || target == "statistics") {
        PrintStatsHelp(history);
    }
    else if (target == "plot" || target == "plotting") {
        PrintPlotHelp(history);
    }
    else if (target == "units") {
        PrintUnitsHelp(history);
    }
    else if (target == "symbolic") {
        PrintSymbolicHelp(history);
    }
    else if (target == "modes") {
        PrintGeneralHelp(history);
    }
    else {
        history.push_back({"Error: Unknown help topic '" + target + "'", Color::Red, true});
        history.push_back({"Available help topics:", Color::White, false});
        history.push_back({"  help, algebraic, linear, stats, plot, units, symbolic", Color::Cyan, false});
    }
}

void AddResultToHistory(const EngineResult& result, std::vector<HistoryLine>& history) {
    if (result.error.has_value()) {
        std::visit([&](auto&& err) {
            using T = std::decay_t<decltype(err)>;
            if constexpr (std::is_same_v<T, CalcErr>) {
                std::string msg = "Error: ";
                switch (err) {
                    case CalcErr::DivideByZero: msg += "Division by Zero!"; break;
                    case CalcErr::NegativeRoot: msg += "Negative Root!"; break;
                    case CalcErr::DomainError: msg += "Domain Error!"; break;
                    case CalcErr::OperationNotFound: msg += "Operation Not Found!"; break;
                    case CalcErr::ArgumentMismatch: msg += "Syntax/Argument Error!"; break;
                    case CalcErr::NumericOverflow: msg += "Numeric Overflow!"; break;
                    case CalcErr::StackOverflow: msg += "Stack Overflow!"; break;
                    case CalcErr::MemoryExhausted: msg += "Memory Exhausted!"; break;
                    case CalcErr::InfiniteLoop: msg += "Infinite Loop Detected!"; break;
                    default: msg += "Unknown Algebraic Error!"; break;
                }
                history.push_back({msg, Color::Red, true});
            } else if constexpr (std::is_same_v<T, LinAlgErr>) {
                std::string msg = "Error: ";
                switch (err) {
                    case LinAlgErr::NoSolution: msg += "Singular Matrix / No Unique Solution!"; break;
                    case LinAlgErr::MatrixMismatch: msg += "Matrix Dimensions Mismatch!"; break;
                    case LinAlgErr::ParseError: msg += "Input Parsing Failed!"; break;
                    default: msg += "Unknown Linear Algebra Error!"; break;
                }
                history.push_back({msg, Color::Red, true});
            }
        }, result.error.value());
    } 
    else if (result.result.has_value()) {
        std::visit([&](auto&& res) {
            using T = std::decay_t<decltype(res)>;
            
            if constexpr (std::is_same_v<T, double>) {
                history.push_back({"= " + std::to_string(res), Color::Green, true});
            } 
            else if constexpr (std::is_same_v<T, std::vector<double>>) {
                history.push_back({"Result Vector:", Color::Yellow, false});
                std::string s = "[ ";
                for (double d : res) s += std::to_string(d) + " ";
                s += "]";
                history.push_back({s, Color::Cyan, false});
            } 
            else if constexpr (std::is_same_v<T, Matrix>) {
                history.push_back({"Matrix Result:", Color::Yellow, false});
                for (const auto& row : res) {
                    std::string s = "  [ ";
                    for (double val : row) s += std::to_string(val) + " ";
                    s += "]";
                    history.push_back({s, Color::Cyan, false});
                }
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                // Check if this is a plot result (contains newlines, indicating ASCII art)
                if (res.find('\n') != std::string::npos && res.length() > 50) {
                    // This looks like a plot result - display it directly
                    std::stringstream ss(res);
                    std::string line;
                    while (std::getline(ss, line)) {
                        history.push_back({line, Color::Cyan, false});
                    }
                } else {
                    // Regular symbolic result
                    history.push_back({"Symbolic Result:", Color::Yellow, false});
                    history.push_back({"  " + res, Color::Cyan, true});
                }
            }
        }, result.result.value());
    }
}

int main(int argc, char* argv[]) {
    // 🏎️ SENNA SPEED MODE - Ultra-fast batch execution with enhanced architecture! 🏎️
    if (argc > 1) {
        // Initialize enhanced architecture
        OGulator::CPUOptimization::Initialize();
        OGulator::Dispatch::Initialize();
        OGulator::Dispatch::OptimizeForSpeed();
        
        // Batch mode for lightning-fast command execution
        std::string command;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) command += " ";
            command += argv[i];
        }
        
        try {
            // Use selective dispatcher for optimal performance
            auto result = OGulator::Dispatch::Calculate(command);
            
            // Check if we got a successful result
            if (result.result.has_value()) {
                auto& success_result = result.result.value();
                
                // Handle different result types
                if (std::holds_alternative<double>(success_result)) {
                    std::cout << std::fixed << std::setprecision(6) << std::get<double>(success_result) << std::endl;
                } else if (std::holds_alternative<std::string>(success_result)) {
                    std::cout << std::get<std::string>(success_result) << std::endl;
                } else {
                    std::cout << "Complex result" << std::endl;
                }
            } else if (result.error.has_value()) {
                std::cerr << "Calculation error occurred" << std::endl;
                return 1;
            } else {
                std::cerr << "Unknown error" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        
        // Cleanup
        OGulator::Dispatch::Shutdown();
        return 0;
    }
    
    // Interactive TUI mode with enhanced architecture
    
    // Initialize enhanced computing architecture
    OGulator::CPUOptimization::Initialize();
    OGulator::Dispatch::Initialize();
    OGulator::Dispatch::OptimizeForSpeed();
    
    CalcEngine engine;
    std::function<void()> exit_app; 
    
    std::string input_buffer;
    int cursor_position = 0;
    
    std::vector<HistoryLine> history;
    int scroll_offset = 0;
    
    std::vector<std::string> command_history;
    int history_index = 0;

    std::string mode_str = "ALGEBRAIC";
    double last_ans_value = 0.0;

#ifdef ENABLE_PYTHON_FFI
    std::unique_ptr<PythonREPL> python_repl;
    bool python_interactive_mode = false;
    
    // Initialize Python REPL if Python engine is available
    if (engine.GetPythonEngine()) {
        python_repl = std::make_unique<PythonREPL>(engine.GetPythonEngine());
    }
#endif

    history.push_back({"OGULATOR v" PROJECT_VER_STRING " (Enhanced) - Ready", Color::White, true});
    history.push_back({"Build: " BUILD_TIMESTAMP, Color::GrayLight, false});
    history.push_back({"🚀 Enhanced Architecture: Eigen + Nanobind + Dispatcher", Color::Green, true});
    history.push_back({"Type 'help' for commands.", Color::GrayLight, false});

    InputOption option;
    option.placeholder = "Type expression here...";
    option.content = &input_buffer;
    option.cursor_position = &cursor_position;
    
    auto input_component = Input(option);

    auto component = CatchEvent(input_component, [&](Event event) {
        if (event == Event::Return) {
            std::string raw_input = input_buffer;
            std::string cmd = Utils::Trim(raw_input);

            if (cmd.empty()) return true;

            // Implicit Ans: If operator is first char, prepend 'Ans'
            if (!cmd.empty()) {
                char first = cmd[0];
                if (first == '+' || first == '-' || first == '*' || first == '/' || first == '^' || first == '%') {
                    cmd = "Ans" + cmd;
                }
            }

            // [FIXED] Removed Utils::ReplaceAns call.
            // Ans is now handled via Context injection.

            history.push_back({">> " + raw_input, Color::White, false});
            
            if (command_history.empty() || command_history.back() != raw_input) {
                command_history.push_back(raw_input);
            }
            history_index = command_history.size();
            scroll_offset = 0; 

            if (cmd == "exit") {
                if (exit_app) exit_app();
                else exit(0);
                return true;
            }
            else if (cmd == "clear") {
                history.clear();
                history.push_back({"Screen Cleared", Color::GrayLight, false});
                last_ans_value = 0.0; 
            }
            else if (cmd.rfind("help", 0) == 0) {
                std::string help_topic = "";
                if (cmd.length() > 5) {
                    help_topic = Utils::Trim(cmd.substr(4)); // Remove "help" and trim
                }
                PrintHelp(history, help_topic);
            }
            else if (cmd.rfind("mode ", 0) == 0) {
                if (cmd == "mode linear") {
                    engine.SetMode(CalcMode::LinearSystem);
                    mode_str = "LINEAR SYS";
                    history.push_back({"Switched to LINEAR SYSTEM", Color::Yellow, true});
                } else if (cmd == "mode stats") {
                    engine.SetMode(CalcMode::Statistics);
                    mode_str = "STATISTICS";
                    history.push_back({"Switched to STATISTICS mode", Color::Yellow, true});
                } else if (cmd == "mode symbolic") {
                    engine.SetMode(CalcMode::Symbolic);
                    mode_str = "SYMBOLIC";
                    history.push_back({"Switched to SYMBOLIC mode", Color::Yellow, true});
                } else if (cmd == "mode plot") {
                    engine.SetMode(CalcMode::Plotting);
                    mode_str = "PLOTTING";
                    history.push_back({"Switched to PLOTTING mode", Color::Yellow, true});
                } else if (cmd == "mode units") {
                    engine.SetMode(CalcMode::Units);
                    mode_str = "UNITS";
                    history.push_back({"Switched to UNITS mode", Color::Yellow, true});
#ifdef ENABLE_PYTHON_FFI
                } else if (cmd == "mode python") {
                    engine.SetMode(CalcMode::Python);
                    mode_str = "PYTHON";
                    history.push_back({"Switched to PYTHON mode", Color::Yellow, true});
                } else if (cmd == "python" || cmd == "py") {
                    if (python_repl && engine.GetPythonEngine()) {
                        python_interactive_mode = true;
                        python_repl->StartInteractiveSession();
                        mode_str = "PYTHON REPL";
                        history.push_back({"=== Python Interactive REPL Mode ===", Color::Cyan, true});
                        history.push_back({"Type 'exit()' or 'quit()' to return to calculator", Color::GrayLight, false});
                        history.push_back({">>> ", Color::Green, false});
                    } else {
                        history.push_back({"Python FFI not available", Color::Red, false});
                    }
                } else if (cmd == "mode numpy") {
                    engine.SetMode(CalcMode::PythonNumPy);
                    mode_str = "NUMPY";
                    history.push_back({"Switched to NUMPY mode", Color::Yellow, true});
                } else if (cmd == "mode scipy") {
                    engine.SetMode(CalcMode::PythonSciPy);
                    mode_str = "SCIPY";
                    history.push_back({"Switched to SCIPY mode", Color::Yellow, true});
                } else if (cmd == "mode matplotlib") {
                    engine.SetMode(CalcMode::PythonMatplotlib);
                    mode_str = "MATPLOTLIB";
                    history.push_back({"Switched to MATPLOTLIB mode", Color::Yellow, true});
                } else if (cmd == "mode pandas") {
                    engine.SetMode(CalcMode::PythonPandas);
                    mode_str = "PANDAS";
                    history.push_back({"Switched to PANDAS mode", Color::Yellow, true});
                } else if (cmd == "mode sympy") {
                    engine.SetMode(CalcMode::PythonSymPy);
                    mode_str = "SYMPY";
                    history.push_back({"Switched to SYMPY mode", Color::Yellow, true});
#endif
                } else {
                    engine.SetMode(CalcMode::Algebraic);
                    mode_str = "ALGEBRAIC";
                    history.push_back({"Switched to ALGEBRAIC", Color::Yellow, true});
                }
            }
            else {
#ifdef ENABLE_PYTHON_FFI
                // Handle Python REPL mode
                if (python_interactive_mode && python_repl) {
                    std::string repl_result = python_repl->ExecuteInteractive(cmd);
                    
                    if (!python_repl->IsSessionActive()) {
                        // Exit REPL mode
                        python_interactive_mode = false;
                        mode_str = "ALGEBRAIC";
                        engine.SetMode(CalcMode::Algebraic);
                        history.push_back({"=== Exited Python REPL ===", Color::Cyan, true});
                    } else {
                        // Display REPL result
                        if (!repl_result.empty()) {
                            if (repl_result.substr(0, 4) == ">>> " || repl_result.substr(0, 4) == "... ") {
                                // This is a prompt, no output to show
                                history.push_back({repl_result, Color::Green, false});
                            } else {
                                // This is output
                                history.push_back({repl_result, Color::White, false});
                                history.push_back({">>> ", Color::Green, false});
                            }
                        }
                    }
                } else {
#endif
                    // [NEW] Inject 'Ans' as a variable in the context
                    std::map<std::string, double> context;
                    context["Ans"] = last_ans_value;

                    // Call Engine
                    auto result = engine.EvaluateWithContext(cmd, context);
                    AddResultToHistory(result, history);

                    if (result.result.has_value()) {
                        std::visit([&](auto&& res) {
                            using T = std::decay_t<decltype(res)>;
                            if constexpr (std::is_same_v<T, double>) {
                                last_ans_value = res;
                            }
                        }, result.result.value());
                    }
#ifdef ENABLE_PYTHON_FFI
                }
#endif
            }

            input_buffer = "";
            cursor_position = 0;
            return true;
        }
        // ... (Arrow key handling same as before) ...
        else if (event == Event::ArrowUp) {
            if (command_history.empty()) return true;
            if (history_index > 0) {
                history_index--;
                input_buffer = command_history[history_index];
                cursor_position = input_buffer.size();
            }
            return true;
        }
        else if (event == Event::ArrowDown) {
            if (command_history.empty()) return true;
            if (history_index < command_history.size()) {
                history_index++;
                if (history_index == command_history.size()) input_buffer = "";
                else input_buffer = command_history[history_index];
                cursor_position = input_buffer.size();
            }
            return true;
        }
        else if (event == Event::PageUp) { scroll_offset++; return true; }
        else if (event == Event::PageDown) { if (scroll_offset > 0) scroll_offset--; return true; }

        return false;
    });

    auto renderer = Renderer(component, [&] {
        Elements history_elements;
        int visible_lines = 16; 
        int total_lines = history.size();
        if (scroll_offset > total_lines - visible_lines) scroll_offset = std::max(0, total_lines - visible_lines);
        
        for (int i = std::max(0, total_lines - visible_lines - scroll_offset); i < total_lines - scroll_offset; ++i) {
            auto& line = history[i];
            auto text_el = text(line.content) | color(line.color);
            if (line.is_bold) text_el |= bold;
            history_elements.push_back(text_el);
        }

        return vbox({
            hbox({
                text(" OGULATOR ") | bold | color(Color::Black) | bgcolor(Color::Cyan),
                filler(),
                text(" MODE: " + mode_str + " ") | color(Color::White) | bgcolor(Color::Blue)
            }),
            separator(),
            vbox(history_elements) | flex,
            separator(),
            hbox({
                text(" >> ") | bold | color(Color::Yellow),
                component->Render() | color(Color::White)
            }) 
        }) | border;
    });

    auto screen = ScreenInteractive::Fullscreen();
    exit_app = screen.ExitLoopClosure(); 
    screen.Loop(renderer);

    return 0;
}