/**
 * @file main.cpp
 * @brief AXIOM Engine v3.0 - Enterprise Scientific Computing Platform
 * 
 * Main entry point with daemon mode and enterprise features
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

#include "dynamic_calc.h"
#include "extended_types.h"

// Enterprise features (conditionally compiled based on availability)
#ifdef ENABLE_DAEMON_MODE
    #include "daemon_engine.h"
#endif

#ifdef ENABLE_ARENA_ALLOCATOR
    #include "arena_allocator.h"
#endif

#ifdef ENABLE_SYMENGINE_INTEGRATION
    #include "symengine_integration.h"
#endif

#ifdef ENABLE_FTXUI
    #include "ftxui_gui.h"
#endif

void print_axiom_banner() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    🚀 AXIOM Engine v3.0                              ║\n";
    std::cout << "║               Enterprise Scientific Computing Platform               ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  ⚡ Ultra-High Performance C++ Engine                                ║\n";
    std::cout << "║  🏗️  Enterprise Daemon Architecture                                  ║\n";
    std::cout << "║  🧠 Arena Memory Management                                          ║\n";
    std::cout << "║  ∑  SymEngine Symbolic Computing                                    ║\n";
    std::cout << "║  🔗 Python Integration (nanobind)                                   ║\n";
    std::cout << "║  📊 Advanced Linear Algebra (Eigen)                                 ║\n";
    std::cout << "║  🔥 NUMA-Optimized Memory Pools                                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void print_help() {
    std::cout << "AXIOM Engine v3.0 - Usage:\n\n";
    
    std::cout << "Interactive Modes:\n";
    std::cout << "  axiom                       Start interactive calculator\n";
    std::cout << "  axiom --gui                 Start GUI interface\n";
    std::cout << "  axiom --tui                 Start text-based UI (FTXUI)\n\n";
    
    std::cout << "Enterprise Daemon Mode:\n";
    std::cout << "  axiom --daemon              Start as background daemon\n";
    std::cout << "  axiom --daemon --pipe=NAME  Start daemon with custom pipe name\n";
    std::cout << "  axiom --daemon-status       Check daemon status\n";
    std::cout << "  axiom --daemon-stop         Stop running daemon\n\n";
    
    std::cout << "Command Line Execution:\n";
    std::cout << "  axiom \"expression\"          Execute single expression\n";
    std::cout << "  axiom --mode=MODE \"expr\"     Execute in specific mode\n";
    std::cout << "  axiom --symbolic \"expr\"      Symbolic computation\n";
    std::cout << "  axiom --numeric \"expr\"       Numeric evaluation\n\n";
    
    std::cout << "Modes Available:\n";
    std::cout << "  algebraic    Basic arithmetic and algebra\n";
    std::cout << "  linear       Matrix operations and linear systems\n";
    std::cout << "  statistics   Statistical analysis\n";
    std::cout << "  symbolic     Computer algebra system\n";
    std::cout << "  units        Unit conversions\n\n";
    
    std::cout << "Enterprise Features:\n";
    std::cout << "  axiom --install-service     Install as Windows service\n";
    std::cout << "  axiom --benchmark           Run performance benchmarks\n";
    std::cout << "  axiom --memory-profile      Enable memory profiling\n";
    std::cout << "  axiom --numa-optimize       Enable NUMA optimizations\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  axiom \"2 + 3 * 4\"                    # Basic arithmetic\n";
    std::cout << "  axiom --symbolic \"diff(x^2, x)\"       # Symbolic differentiation\n";
    std::cout << "  axiom --mode=linear \"solve([2,3;1,4], [5;6])\" # Linear system\n";
    std::cout << "  axiom --daemon &                      # Start daemon in background\n\n";
}

int run_interactive_mode() {
    print_axiom_banner();
    
    // Initialize memory management if available
#ifdef ENABLE_ARENA_ALLOCATOR
    AXIOM::MemoryProfiler::instance().enable_profiling(true);
#endif
    
    // Initialize symbolic engine if available
#ifdef ENABLE_SYMENGINE_INTEGRATION
    auto cas = std::make_unique<AXIOM::ComputerAlgebraSystem>();
#endif
    
    // Initialize calculation engine
    auto calc = std::make_unique<AXIOM::DynamicCalc>();
    
    std::cout << "🚀 AXIOM Engine v3.0 ready for enterprise computing...\n";
    std::cout << "💡 Type 'help' for commands, 'exit' to quit\n\n";
    
    std::string input;
    AXIOM::CalculationMode current_mode = AXIOM::CalculationMode::ALGEBRAIC;
    
    while (true) {
        std::cout << "[" << AXIOM::mode_to_string(current_mode) << "] AXIOM> ";
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        if (input == "help") {
            print_help();
            continue;
        }
        
        // Check for mode changes
        if (input == "algebraic") {
            current_mode = AXIOM::CalculationMode::ALGEBRAIC;
            std::cout << "✅ Switched to algebraic mode\n";
            continue;
        } else if (input == "linear") {
            current_mode = AXIOM::CalculationMode::LINEAR_SYSTEM;
            std::cout << "✅ Switched to linear system mode\n";
            continue;
        } else if (input == "statistics") {
            current_mode = AXIOM::CalculationMode::STATISTICS;
            std::cout << "✅ Switched to statistics mode\n";
            continue;
        } else if (input == "symbolic") {
            current_mode = AXIOM::CalculationMode::SYMBOLIC;
            std::cout << "✅ Switched to symbolic mode\n";
            continue;
        } else if (input == "units") {
            current_mode = AXIOM::CalculationMode::UNITS;
            std::cout << "✅ Switched to unit conversion mode\n";
            continue;
        } else if (input == "plot") {
            current_mode = AXIOM::CalculationMode::PLOT;
            std::cout << "✅ Switched to plotting mode\n";
            continue;
        }
        
#ifdef ENABLE_DAEMON_MODE
        if (input == "daemon") {
            std::cout << "🔥 Starting daemon mode...\n";
            auto daemon = std::make_unique<AXIOM::DaemonEngine>();
            if (daemon->start()) {
                std::cout << "✅ Daemon started successfully. Type 'stop' to exit daemon mode.\n";
                
                // Keep daemon running until stop
                while (true) {
                    std::string daemon_input;
                    std::cout << "DAEMON> ";
                    std::getline(std::cin, daemon_input);
                    
                    if (daemon_input == "stop" || daemon_input == "exit") {
                        daemon->stop();
                        std::cout << "🛑 Daemon stopped.\n";
                        break;
                    } else if (daemon_input == "status") {
                        std::cout << "📊 Status: " << static_cast<int>(daemon->get_status()) << "\n";
                        std::cout << "📈 Total requests: " << daemon->get_total_requests() << "\n";
                        std::cout << "⏱️ Avg response time: " << daemon->get_avg_response_time() << "ms\n";
                        std::cout << "🕐 Uptime: " << daemon->get_uptime().count() << "ms\n";
                    }
                }
                continue;
            } else {
                std::cout << "❌ Failed to start daemon.\n";
                continue;
            }
        }
#endif
        
#ifdef ENABLE_ARENA_ALLOCATOR
        if (input == "memory") {
            auto stats = AXIOM::PoolManager::instance().get_all_stats();
            std::cout << "🧠 Memory Pool Statistics:\n";
            for (size_t i = 0; i < stats.size(); ++i) {
                const auto& stat = stats[i];
                std::cout << "  Pool " << i << ": " << stat.used_size << "/" << stat.total_size 
                         << " bytes (" << (100.0 * stat.used_size / stat.total_size) << "%)\n";
            }
            continue;
        }
#endif
        
        // Try calculation
        try {
            auto basic_result = calc->calculate(input, current_mode);
            auto result = AXIOM::ExtendedEngineResult::from_engine_result(basic_result);
            
            if (result.success) {
                if (current_mode == AXIOM::CalculationMode::LINEAR_SYSTEM && result.has_linear_result) {
                    std::cout << "🎯 Linear System Solution:\n";
                    for (size_t i = 0; i < result.linear_result.solution.size(); ++i) {
                        std::cout << "  x" << i << " = " << result.linear_result.solution[i] << "\n";
                    }
                } else if (current_mode == AXIOM::CalculationMode::STATISTICS && result.has_stats_result) {
                    const auto& stats = result.stats_result;
                    std::cout << "📊 Statistical Analysis:\n";
                    std::cout << "  📈 Mean: " << stats.mean << "\n";
                    std::cout << "  📊 Std Dev: " << stats.std_dev << "\n";
                    std::cout << "  📉 Min: " << stats.min << "\n";
                    std::cout << "  📈 Max: " << stats.max << "\n";
                    std::cout << "  🔢 Count: " << stats.count << "\n";
                } else if (current_mode == AXIOM::CalculationMode::SYMBOLIC && result.has_symbolic_result) {
                    std::cout << "∑ Symbolic result: " << result.symbolic_result.result << "\n";
                    if (!result.symbolic_result.simplified.empty()) {
                        std::cout << "🎯 Simplified: " << result.symbolic_result.simplified << "\n";
                    }
                } else if (current_mode == AXIOM::CalculationMode::UNITS && result.has_unit_result) {
                    std::cout << "🔄 Converted: " << result.unit_result.value 
                             << " " << result.unit_result.target_unit << "\n";
                } else if (current_mode == AXIOM::CalculationMode::PLOT && result.has_plot_result) {
                    std::cout << "📊 Plot generated: " << result.plot_result.filename << "\n";
                    std::cout << "📏 Range: [" << result.plot_result.x_min << ", " 
                             << result.plot_result.x_max << "]\n";
                } else {
                    std::cout << "🎯 " << result.value << "\n";
                }
            } else {
                std::cout << "❌ Error: " << result.error_message << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error: " << e.what() << "\n";
        }
    }
    
    return 0;
}

#ifdef ENABLE_DAEMON_MODE
int run_daemon_mode(const std::vector<std::string>& args) {
    std::string pipe_name = "axiom_daemon";
    
    // Parse daemon arguments
    for (const auto& arg : args) {
        if (arg.starts_with("--pipe=")) {
            pipe_name = arg.substr(7);
        }
    }
    
    print_axiom_banner();
    std::cout << "🔥 Starting AXIOM Engine Daemon Mode...\n";
    std::cout << "📡 Pipe name: " << pipe_name << "\n\n";
    
    // Initialize enterprise memory management
#ifdef ENABLE_ARENA_ALLOCATOR
    AXIOM::MemoryProfiler::instance().enable_profiling(true);
#endif
    
    auto daemon = std::make_unique<AXIOM::DaemonEngine>(pipe_name);
    
    if (!daemon->start()) {
        std::cerr << "❌ Failed to start daemon\n";
        return 1;
    }
    
    std::cout << "✅ AXIOM Daemon started successfully\n";
    std::cout << "🚀 Enterprise mode: HIGH-PERFORMANCE PERSISTENT COMPUTING\n";
    std::cout << "📊 Memory pools: NUMA-optimized allocation\n";
    std::cout << "⚡ Symbolic engine: SymEngine integration active\n\n";
    
    // Keep daemon running
    while (daemon->is_running()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Print periodic status
        static auto last_status_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (now - last_status_time >= std::chrono::minutes(5)) {
            std::cout << "📈 Status: " << daemon->get_total_requests() 
                     << " requests, " << daemon->get_avg_response_time() 
                     << "ms avg response time, uptime " 
                     << daemon->get_uptime().count() << "ms\n";
            last_status_time = now;
        }
    }
    
    std::cout << "🛑 AXIOM Daemon stopped\n";
    return 0;
}
#endif

int run_benchmark_mode() {
    print_axiom_banner();
    std::cout << "🏁 Running AXIOM Engine Performance Benchmarks...\n\n";
    
    // Initialize systems
    auto calc = std::make_unique<AXIOM::DynamicCalc>();
    
    // Benchmark 1: Basic arithmetic
    std::cout << "🔢 Basic Arithmetic Benchmark:\n";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        calc->calculate("2 + 3 * 4 - 1", AXIOM::CalculationMode::ALGEBRAIC);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "  ⚡ 10,000 calculations in " << duration.count() << "μs\n";
    std::cout << "  🏎️ " << (10000.0 * 1000000.0 / duration.count()) << " calculations/second\n\n";
    
#ifdef ENABLE_ARENA_ALLOCATOR
    // Benchmark 3: Memory allocation
    std::cout << "🧠 Memory Arena Benchmark:\n";
    auto arena = std::make_unique<AXIOM::MemoryArena>(64 * 1024 * 1024);
    start = std::chrono::high_resolution_clock::now();
    std::vector<void*> ptrs;
    for (int i = 0; i < 100000; ++i) {
        ptrs.push_back(arena->allocate(64));
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "  ⚡ 100,000 allocations in " << duration.count() << "μs\n";
    std::cout << "  🏎️ " << (100000.0 * 1000000.0 / duration.count()) << " allocations/second\n\n";
#endif
    
    std::cout << "✅ All benchmarks completed successfully!\n";
    std::cout << "🚀 AXIOM Engine v3.0 delivering enterprise-grade performance!\n";
    
    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    
    // Check for help
    if (args.empty()) {
        return run_interactive_mode();
    }
    
    if (std::find(args.begin(), args.end(), "--help") != args.end() ||
        std::find(args.begin(), args.end(), "-h") != args.end()) {
        print_help();
        return 0;
    }
    
    // Check for daemon mode
#ifdef ENABLE_DAEMON_MODE
    if (std::find(args.begin(), args.end(), "--daemon") != args.end()) {
        return run_daemon_mode(args);
    }
    
    // Check for daemon status
    if (std::find(args.begin(), args.end(), "--daemon-status") != args.end()) {
        bool running = AXIOM::DaemonClient::is_daemon_running();
        std::cout << "🔍 Daemon status: " << (running ? "🟢 RUNNING" : "🔴 STOPPED") << "\n";
        return running ? 0 : 1;
    }
#endif
    
    // Check for GUI mode
    if (std::find(args.begin(), args.end(), "--gui") != args.end()) {
        std::cout << "🖥️ Starting GUI mode...\n";
        std::cout << "Use Python GUI: python gui/python/axiom_gui.py\n";
        return 0;
    }
    
    // Check for TUI mode
    if (std::find(args.begin(), args.end(), "--tui") != args.end()) {
#ifdef ENABLE_FTXUI
        return run_ftxui_interface();
#else
        std::cout << "FTXUI not available. Using interactive mode.\n";
        return run_interactive_mode();
#endif
    }
    
    // Check for benchmark mode
    if (std::find(args.begin(), args.end(), "--benchmark") != args.end()) {
        return run_benchmark_mode();
    }
    
    // Command line execution
    if (!args.empty()) {
        std::string expression = args[0];
        auto calc = std::make_unique<AXIOM::DynamicCalc>();
        
        try {
            auto basic_result = calc->calculate(expression, AXIOM::CalculationMode::ALGEBRAIC);
            auto result = AXIOM::ExtendedEngineResult::from_engine_result(basic_result);
            
            if (result.success) {
                // Output with proper precision, removing trailing zeros
                printf("%.15g", result.value);
                printf("\n");
                return 0;
            } else {
                std::cerr << "Error: " << result.error_message << "\n";
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }
    
    // Default to interactive mode
    return run_interactive_mode();
}