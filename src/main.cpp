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

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

#include "dynamic_calc.h"
#include "string_helpers.h"
#include "version.h"

using namespace ftxui;

struct HistoryLine {
    std::string content;
    Color color;
    bool is_bold;
};

void PrintHelp(std::vector<HistoryLine>& history, const std::string& target) {
    if (target.empty() || target == "general") {
        history.push_back({"--- OGULATOR COMMANDS ---", Color::Cyan, true});
        history.push_back({"[NAVIGATION]", Color::Yellow, true});
        history.push_back({"  UP/DOWN Arrow : Cycle Command History", Color::White, false});
        history.push_back({"  PgUp/PgDn     : Scroll Output Log", Color::White, false});
        history.push_back({"[GLOBAL]", Color::Yellow, true});
        history.push_back({"  help   : Show this menu", Color::White, false});
        history.push_back({"  clear  : Clear screen", Color::White, false});
        history.push_back({"  exit   : Close application", Color::White, false});
        history.push_back({"[MODES]", Color::Yellow, true});
        history.push_back({"  mode algebraic : Scientific calc", Color::White, false});
        history.push_back({"  mode linear    : Matrix/System solver", Color::White, false});
        history.push_back({"  mode stats     : Statistical analysis", Color::White, false});
        history.push_back({"  mode symbolic  : Symbolic math (placeholder)", Color::White, false});
        history.push_back({"  mode plot      : Function plotting", Color::White, false});
        history.push_back({"  mode units     : Unit conversions", Color::White, false});
        history.push_back({"[SPECIAL COMMANDS]", Color::Yellow, true});
        history.push_back({"  plot sin(x)    : Plot function (any mode)", Color::White, false});
        history.push_back({"  convert 5 m to ft : Unit conversion", Color::White, false});
        history.push_back({"  derive x^2     : Symbolic derivative", Color::White, false});
        history.push_back({"-------------------------", Color::Cyan, false});
        return;
    }
    history.push_back({"Error: Unknown help topic.", Color::Red, true});
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
                history.push_back({"Symbolic Result:", Color::Yellow, false});
                history.push_back({"  " + res, Color::Cyan, true});
            }
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
    double last_ans_value = 0.0;

    history.push_back({"OGULATOR v" PROJECT_VER_STRING " (Symbolic) - Ready", Color::White, true});
    history.push_back({"Build: " BUILD_TIMESTAMP, Color::GrayLight, false});
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
            else if (cmd == "help") {
                PrintHelp(history, "");
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
                } else {
                    engine.SetMode(CalcMode::Algebraic);
                    mode_str = "ALGEBRAIC";
                    history.push_back({"Switched to ALGEBRAIC", Color::Yellow, true});
                }
            }
            else {
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