// dynamic_calc.cpp
// This file implements the core calculator engine, which manages different calculation modes
// and delegates input parsing and execution to the appropriate parser.

#include "dynamic_calc.h"
#include "algebraic_parser.h"
#include "linear_system_parser.h"
#include "unit_parser.h"
#ifdef ENABLE_PYTHON_FFI
#include "python_parser.h"
#endif
#include <stdexcept>

// Constructor: Initializes the calculator engine and registers parsers for each mode.
CalcEngine::CalcEngine() {
    parsers_[CalcMode::Algebraic] = std::make_unique<AlgebraicParser>(); // Algebraic mode parser.
    parsers_[CalcMode::LinearSystem] = std::make_unique<LinearSystemParser>(); // Linear system mode parser.
    
    // Initialize new engines
    unit_manager_ = std::make_unique<UnitManager>();
    symbolic_engine_ = std::make_unique<SymbolicEngine>();
    statistics_engine_ = std::make_unique<StatisticsEngine>();
    plot_engine_ = std::make_unique<PlotEngine>();
    
    // Initialize unit parser
    parsers_[CalcMode::Units] = std::make_unique<UnitParser>(unit_manager_.get());
    
#ifdef ENABLE_PYTHON_FFI
    // Initialize Python engine and parsers only if Python is available
    python_engine_ = std::make_unique<PythonEngine>();
    
    // Initialize Python parsers for different modes
    parsers_[CalcMode::Python] = std::make_unique<PythonParser>(python_engine_.get(), PythonMode::Interactive);
    parsers_[CalcMode::PythonNumPy] = std::make_unique<PythonParser>(python_engine_.get(), PythonMode::NumPy);
    parsers_[CalcMode::PythonSciPy] = std::make_unique<PythonParser>(python_engine_.get(), PythonMode::SciPy);
    parsers_[CalcMode::PythonMatplotlib] = std::make_unique<PythonParser>(python_engine_.get(), PythonMode::Matplotlib);
    parsers_[CalcMode::PythonPandas] = std::make_unique<PythonParser>(python_engine_.get(), PythonMode::Pandas);
    parsers_[CalcMode::PythonSymPy] = std::make_unique<PythonParser>(python_engine_.get(), PythonMode::SymPy);
#endif
}

// Sets the current calculation mode.
// @param mode: The calculation mode to set (e.g., Algebraic, LinearSystem).
void CalcEngine::SetMode(CalcMode mode) {
    current_mode_ = mode;
}

// Evaluates the given input string using the current calculation mode.
// @param input: The input string to evaluate.
// @return: The result of the evaluation as an EngineResult.
EngineResult CalcEngine::Evaluate(const std::string &input) {
   return EvaluateWithContext(input,{});
}

EngineResult CalcEngine::EvaluateWithContext(const std::string& input,const std::map<std::string,double>& context){
    // DEBUG: Test if this function is called at all
    if (input == "test") {
        return {EngineSuccessResult("DEBUG: EvaluateWithContext called"), {}};
    }
    
    // Handle special commands that work across all modes
    if (input.find("plot(") == 0) {
        // Parse plot(expression, x_min, x_max, y_min, y_max)
        size_t open_paren = input.find('(');
        size_t close_paren = input.rfind(')');
        
        if (open_paren != std::string::npos && close_paren != std::string::npos && close_paren > open_paren) {
            std::string args_content = input.substr(open_paren + 1, close_paren - open_paren - 1);
            
            // Simple comma splitting 
            std::vector<std::string> parts;
            std::string current_part;
            int paren_count = 0;
            
            for (char c : args_content) {
                if (c == '(') paren_count++;
                else if (c == ')') paren_count--;
                else if (c == ',' && paren_count == 0) {
                    if (!current_part.empty()) {
                        // Trim whitespace
                        size_t start = current_part.find_first_not_of(" \t");
                        size_t end = current_part.find_last_not_of(" \t");
                        if (start != std::string::npos && end != std::string::npos) {
                            parts.push_back(current_part.substr(start, end - start + 1));
                        }
                    }
                    current_part.clear();
                    continue;
                }
                current_part += c;
            }
            
            // Add the last part
            if (!current_part.empty()) {
                size_t start = current_part.find_first_not_of(" \t");
                size_t end = current_part.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos) {
                    parts.push_back(current_part.substr(start, end - start + 1));
                }
            }
            
            if (parts.size() == 5) {
                try {
                    std::string expression = parts[0];
                    double x_min = std::stod(parts[1]);
                    double x_max = std::stod(parts[2]);
                    double y_min = std::stod(parts[3]);
                    double y_max = std::stod(parts[4]);
                    
                    PlotConfig config;
                    config.x_min = x_min;
                    config.x_max = x_max;
                    config.y_min = y_min;
                    config.y_max = y_max;
                    config.width = 60;
                    config.height = 20;
                    config.show_axes = true;
                    config.plot_char = '*';
                    
                    std::string plot_result = plot_engine_->PlotFunction(expression, config);
                    return {EngineSuccessResult(plot_result), {}};
                    
                } catch (const std::exception&) {
                    return {{}, {EngineErrorResult(CalcErr::ArgumentMismatch)}};
                }
            } else {
                // If wrong number of arguments, show a helpful error
                return {{}, {EngineErrorResult(CalcErr::ArgumentMismatch)}};
            }
        }
    }
    
    if (input.find("stats ") == 0 && current_mode_ != CalcMode::Statistics) {
        // Handle basic stats commands like "stats mean [1,2,3,4]"
        return {{}, {CalcErr::OperationNotFound}}; // Placeholder
    }
    
    if (input.find("convert ") == 0 || input.find(" to ") != std::string::npos) {
        auto unit_result = unit_manager_->ConvertUnit(1.0, "m", "ft"); // Parse properly
        return unit_result;
    }
    
    auto it = parsers_.find(current_mode_);
    if(it == parsers_.end()){
        // Special case: Plotting mode doesn't need a separate parser
        if (current_mode_ == CalcMode::Plotting) {
            // In plotting mode, treat input as algebraic expressions for plotting
            auto algebraic_it = parsers_.find(CalcMode::Algebraic);
            if (algebraic_it != parsers_.end()) {
                AlgebraicParser* alg_parser = static_cast<AlgebraicParser*>(algebraic_it->second.get());
                return alg_parser->ParseAndExecuteWithContext(input, context);
            }
        }
        return {{},{EngineErrorResult(CalcErr::OperationNotFound)}};
    }

    if(current_mode_== CalcMode::Algebraic || current_mode_== CalcMode::Plotting){
        CalcMode mode_to_use = (current_mode_ == CalcMode::Plotting) ? CalcMode::Algebraic : current_mode_;
        auto parser_it = parsers_.find(mode_to_use);
        if (parser_it != parsers_.end()) {
            AlgebraicParser* alg_parser = static_cast<AlgebraicParser*>(parser_it->second.get());
            return alg_parser->ParseAndExecuteWithContext(input, context);
        }
    }
    
    return it->second->ParseAndExecute(input);
}
