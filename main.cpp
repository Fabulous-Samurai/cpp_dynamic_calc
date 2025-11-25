#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

#include "dynamic_calc.h"

using namespace ftxui;

struct HistoryLine {
    std::string content;
    Color color;
    bool is_bold;
};

std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

void PrintHelp(std::vector<HistoryLine>& history, const std::string& target) {
    if (target.empty() || target == "general") {
        history.push_back({"--- OGULATOR COMMANDS ---", Color::Cyan, true});
        history.push_back({"[NAVIGATION]", Color::Yellow, true});
        history.push_back({"  UP/DOWN Arrow : Cycle Command History", Color::White, false});
        history.push_back({"  PgUp/PgDn     : Scroll Output Log", Color::White, false});
        history.push_back({"[GLOBAL]", Color::Yellow, true});
        history.push_back({"  help   : Show this menu", Color::White, false});
        history.push_back({"  help modes : List all calculator modes", Color::White, false});
        history.push_back({"  clear  : Clear screen", Color::White, false});
        history.push_back({"  exit   : Close application", Color::White, false});
        history.push_back({"[MODES]", Color::Yellow, true});
        history.push_back({"  mode algebraic : Scientific calc", Color::White, false});
        history.push_back({"  mode linear    : Matrix/System solver", Color::White, false});
        history.push_back({"-------------------------", Color::Cyan, false});
        return;
    }

    if (target == "modes" || target == "help modes") {
        history.push_back({"--- AVAILABLE MODES ---", Color::Cyan, true});
        history.push_back({"[ALGEBRAIC]", Color::Yellow, true});
        history.push_back({"  Scientific RPN/Shunting-Yard math engine.", Color::GrayLight, false});
        history.push_back({"  Run 'help algebraic' for details.", Color::White, false});
        history.push_back({"[LINEAR SYS]", Color::Yellow, true});
        history.push_back({"  Advanced system solving and analysis.", Color::GrayLight, false});
        history.push_back({"  Run 'help linear' for details.", Color::White, false});
        history.push_back({"[COMPLEX]", Color::Yellow, true});
        history.push_back({"  (Beta) Complex number arithmetic.", Color::GrayLight, false});
        history.push_back({"-------------------------", Color::Cyan, false});
        return;
    }

    if (target == "algebraic" || target == "modes algebraic") {
        history.push_back({"--- ALGEBRAIC MODE COMMANDS ---", Color::Cyan, true});
        history.push_back({"[SCIENTIFIC CALC]", Color::Yellow, true});
        history.push_back({"  <expression>      : Evaluates standard math expressions.", Color::White, false});
        history.push_back({"  Supported Fns     : sin, cos, tan, cot, sec, csc (All Arc/Hyp versions included).", Color::GrayLight, false});
        history.push_back({"[SOLVER]", Color::Yellow, true});
        history.push_back({"  quadratic a b c   : Solves ax^2 + bx + c = 0.", Color::White, false});
        history.push_back({"  Ex: quadratic 1 -5 6", Color::GrayLight, false});
        history.push_back({"[NON-LINEAR SOLVER]", Color::Yellow, true});
        history.push_back({"  Uses Newton-Raphson Model to solve higher order non-linear equations", Color::GrayLight, false});
        history.push_back({"  Ex: solve_nl {x^2+y^2=5; x-y=1} [1, 1]", Color::White, false});
        history.push_back({"[SYNTAX]", Color::Yellow, true});
        history.push_back({"  Note: Trigonometry functions expect Degree inputs.", Color::GrayLight, false});
        history.push_back({"-------------------------", Color::Cyan, false});
        return;
    }

    if (target == "linear" || target == "modes linear") {
        history.push_back({"--- LINEAR SYSTEM COMMANDS ---", Color::Cyan, true});
        history.push_back({"[SOLVER]", Color::Yellow, true});
        history.push_back({"  <eqns>   : Solve Ax=b (Gauss-Jordan).", Color::White, false});
        history.push_back({"  cramer   : Solve with Cramer's Rule.", Color::White, false});
        history.push_back({"[ANALYSIS]", Color::Yellow, true});
        history.push_back({"  qr [[..]]: QR Decomposition (Gram-Schmidt).", Color::White, false});
        history.push_back({"  eigen [..]: Eigenvalues/Vectors (QR Algorithm).", Color::White, false});
        history.push_back({"[SYNTAX]", Color::Yellow, true});
        history.push_back({"  Matrix: [[1,2],[3,4]] OR 1 2; 3 4", Color::GrayLight, false});
        history.push_back({"-------------------------", Color::Cyan, false});
        return;
    }

    history.push_back({"Error: Unknown help topic or command.", Color::Red, true});
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
                    default: msg += "Unknown Algebraic Error!"; break;
                }
                history.push_back({msg, Color::Red, true});
            } else if constexpr (std::is_same_v<T, LinAlgErr>) {
                std::string msg = "Error: ";
                switch (err) {
                    case LinAlgErr::NoSolution: msg += "Singular Matrix / No Unique Solution!"; break;
                    case LinAlgErr::InfiniteSolutions: msg += "Infinite Solutions!"; break;
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
                history.push_back({"Result Set / Vector:", Color::Yellow, false});
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
            #ifdef __cpp_lib_complex
            else if constexpr (std::is_same_v<T, std::complex<double>>) {
                std::stringstream ss;
                ss << "(" << res.real() << " + " << res.imag() << "i)";
                history.push_back({ss.str(), Color::Magenta, true});
            }
            #endif
        }, result.result.value());
    }
}

int main() {
    CalcEngine engine;
    std::function<void()> exit_app; 
    
    std::string input_buffer;
    int cursor_position = 0;
    
    std::vector<HistoryLine> history;
    int scroll_offset = 0;
    
    std::vector<std::string> command_history;
    int history_index = 0;

    std::string mode_str = "ALGEBRAIC";

    history.push_back({"OGULATOR v2.4 (Stable) - Ready", Color::White, true});
    history.push_back({"Type 'help' for commands. Use Arrows/PgUp/PgDn.", Color::GrayLight, false});

    InputOption option;
    option.placeholder = "Type expression here...";
    option.content = &input_buffer;
    option.cursor_position = &cursor_position;
    
    auto input_component = Input(option);

    auto component = CatchEvent(input_component, [&](Event event) {
        if (event == Event::Return) {
            std::string raw_input = input_buffer;
            std::string cmd = Trim(raw_input);

            if (cmd.empty()) return true;

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
            }
            else if (cmd == "help") {
                PrintHelp(history, "");
            }
            else if (cmd.rfind("help ", 0) == 0) {
                 std::string target = Trim(cmd.substr(5));
                 PrintHelp(history, target);
            }
            else if (cmd == "mode linear") {
                engine.SetMode(CalcMode::LinearSystem);
                mode_str = "LINEAR SYS";
                history.push_back({"Mode switched to LINEAR SYSTEM", Color::Yellow, true});
            }
            else if (cmd == "mode algebraic") {
                engine.SetMode(CalcMode::Algebraic);
                mode_str = "ALGEBRAIC";
                history.push_back({"Mode switched to ALGEBRAIC", Color::Yellow, true});
            }
            else if (cmd == "mode complex") {
                mode_str = "COMPLEX";
                history.push_back({"Mode switched to COMPLEX (Beta)", Color::Magenta, true});
            }
            else {
                auto result = engine.Evaluate(cmd);
                AddResultToHistory(result, history);
            }

            input_buffer = "";
            cursor_position = 0;
            return true;
        }
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
                if (history_index == command_history.size()) {
                    input_buffer = "";
                } else {
                    input_buffer = command_history[history_index];
                }
                cursor_position = input_buffer.size();
            }
            return true;
        }
        else if (event == Event::PageUp || event.mouse().button == Mouse::WheelUp) {
            scroll_offset++; 
            return true;
        }
        else if (event == Event::PageDown || event.mouse().button == Mouse::WheelDown) {
            if (scroll_offset > 0) scroll_offset--; 
            return true;
        }

        return false;
    });

    auto renderer = Renderer(component, [&] {
        Elements history_elements;
        int visible_lines = 16; 
        int total_lines = history.size();
        
        if (scroll_offset > total_lines - visible_lines) 
            scroll_offset = std::max(0, total_lines - visible_lines);
        
        int start_index = std::max(0, total_lines - visible_lines - scroll_offset);
        int end_index = std::max(0, total_lines - scroll_offset);

        for (int i = start_index; i < end_index && i < total_lines; ++i) {
            auto& line = history[i];
            auto text_el = text(line.content) | color(line.color);
            if (line.is_bold) text_el |= bold;
            history_elements.push_back(text_el);
        }

        return vbox({
            hbox({
                text(" OGULATOR ") | bold | color(Color::Black) | bgcolor(Color::Cyan),
                filler(),
                text(" MODE: " + mode_str + " ") | color(Color::White) | bgcolor(Color::Blue),
                text(" BAT: 98% ") | color(Color::Green)
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