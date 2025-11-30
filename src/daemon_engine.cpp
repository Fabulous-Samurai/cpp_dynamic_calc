/**
 * @file daemon_engine.cpp
 * @brief AXIOM Engine v3.0 - Enterprise Daemon Implementation
 * 
 * Ultra-high performance persistent computation daemon
 */

#include "daemon_engine.h"
#include "dynamic_calc.h"
#include "algebraic_parser.h"
#include "linear_system_parser.h"

#include <sstream>
#include <random>
#include <iomanip>
#include <algorithm>

#ifdef _WIN32
    #include <io.h>
    #include <process.h>
#else
    #include <signal.h>
    #include <sys/wait.h>
#endif

namespace AXIOM {

// ============================================================================
// SessionContext Implementation
// ============================================================================

SessionContext::SessionContext(const std::string& id) 
    : session_id(id)
    , current_mode("algebraic")
    , created_at(std::chrono::steady_clock::now())
    , last_access(std::chrono::steady_clock::now())
{
    // Initialize computation engines
    try {
        algebraic_parser = std::make_unique<AlgebraicParser>();
        linear_parser = std::make_unique<LinearSystemParser>();
        
        history.push_back("Session " + session_id + " initialized at " + 
                         std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                             created_at.time_since_epoch()).count()));
                             
    } catch (const std::exception& e) {
        history.push_back("Error initializing session: " + std::string(e.what()));
    }
}

SessionContext::~SessionContext() = default;

// ============================================================================
// DaemonEngine Implementation  
// ============================================================================

DaemonEngine::DaemonEngine(const std::string& pipe_name)
    : pipe_name_(pipe_name)
    , startup_time_(std::chrono::steady_clock::now())
#ifdef _WIN32
    , pipe_handle_(INVALID_HANDLE_VALUE)
#else
    , pipe_fd_(-1)
#endif
{
}

DaemonEngine::~DaemonEngine() {
    stop();
}

bool DaemonEngine::start() {
    if (running_.load()) {
        return true; // Already running
    }
    
    status_.store(DaemonStatus::STARTING);
    
    if (!setup_pipe()) {
        status_.store(DaemonStatus::DAEMON_ERROR);
        return false;
    }
    
    running_.store(true);
    
    // Start daemon communication thread
    daemon_thread_ = std::thread(&DaemonEngine::daemon_loop, this);
    
    // Start request processor thread
    request_processor_ = std::thread(&DaemonEngine::request_processor_loop, this);
    
    status_.store(DaemonStatus::READY);
    return true;
}

void DaemonEngine::stop() {
    running_.store(false);
    status_.store(DaemonStatus::SHUTDOWN);
    
    // Wake up waiting threads
    queue_cv_.notify_all();
    
    if (daemon_thread_.joinable()) {
        daemon_thread_.join();
    }
    
    if (request_processor_.joinable()) {
        request_processor_.join();
    }
    
    cleanup_pipe();
    
    // Clear sessions
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.clear();
}

bool DaemonEngine::setup_pipe() {
#ifdef _WIN32
    std::string pipe_path = "\\\\.\\pipe\\" + pipe_name_;
    
    pipe_handle_ = CreateNamedPipeA(
        pipe_path.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        4096, 4096, 0, nullptr
    );
    
    return pipe_handle_ != INVALID_HANDLE_VALUE;
#else
    std::string pipe_path = "/tmp/" + pipe_name_;
    
    // Remove existing pipe if it exists
    unlink(pipe_path.c_str());
    
    if (mkfifo(pipe_path.c_str(), 0666) != 0) {
        return false;
    }
    
    pipe_fd_ = open(pipe_path.c_str(), O_RDWR | O_NONBLOCK);
    return pipe_fd_ != -1;
#endif
}

void DaemonEngine::cleanup_pipe() {
#ifdef _WIN32
    if (pipe_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_handle_);
        pipe_handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (pipe_fd_ != -1) {
        close(pipe_fd_);
        pipe_fd_ = -1;
    }
    
    std::string pipe_path = "/tmp/" + pipe_name_;
    unlink(pipe_path.c_str());
#endif
}

void DaemonEngine::daemon_loop() {
    while (running_.load()) {
        try {
#ifdef _WIN32
            // Windows named pipe handling
            if (ConnectNamedPipe(pipe_handle_, nullptr) || GetLastError() == ERROR_PIPE_CONNECTED) {
                char buffer[4096];
                DWORD bytes_read = 0;
                
                if (ReadFile(pipe_handle_, buffer, sizeof(buffer) - 1, &bytes_read, nullptr)) {
                    buffer[bytes_read] = '\\0';
                    
                    // Parse request (simplified JSON-like format)
                    std::string request_str(buffer);
                    Request request;
                    
                    // Simple parsing - in production would use JSON library
                    std::size_t cmd_pos = request_str.find("\"command\":");
                    if (cmd_pos != std::string::npos) {
                        std::size_t start = request_str.find('\"', cmd_pos + 10);
                        std::size_t end = request_str.find('\"', start + 1);
                        if (start != std::string::npos && end != std::string::npos) {
                            request.command = request_str.substr(start + 1, end - start - 1);
                            request.request_id = next_request_id_.fetch_add(1);
                            request.timestamp = std::chrono::steady_clock::now();
                            
                            // Add to queue
                            {
                                std::lock_guard<std::mutex> lock(queue_mutex_);
                                request_queue_.push(request);
                            }
                            queue_cv_.notify_one();
                        }
                    }
                }
                
                DisconnectNamedPipe(pipe_handle_);
            }
#else
            // Linux FIFO handling
            char buffer[4096];
            ssize_t bytes_read = read(pipe_fd_, buffer, sizeof(buffer) - 1);
            
            if (bytes_read > 0) {
                buffer[bytes_read] = '\\0';
                
                std::string request_str(buffer);
                Request request;
                request.command = request_str; // Simplified
                request.request_id = next_request_id_.fetch_add(1);
                request.timestamp = std::chrono::steady_clock::now();
                
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    request_queue_.push(request);
                }
                queue_cv_.notify_one();
            }
#endif
        } catch (const std::exception& e) {
            // Log error and continue
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void DaemonEngine::request_processor_loop() {
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Wait for requests
        queue_cv_.wait(lock, [this] { 
            return !request_queue_.empty() || !running_.load(); 
        });
        
        if (!running_.load()) {
            break;
        }
        
        Request request = request_queue_.front();
        request_queue_.pop();
        lock.unlock();
        
        // Process request
        status_.store(DaemonStatus::BUSY);
        Response response = execute_command(request);
        status_.store(DaemonStatus::READY);
        
        // Update metrics
        update_metrics(response.execution_time_ms);
        total_requests_.fetch_add(1);
        
        // Send response (simplified - in production would queue responses)
    }
}

DaemonEngine::Response DaemonEngine::execute_command(const Request& request) {
    Response response;
    response.request_id = request.request_id;
    response.session_id = request.session_id;
    response.timestamp = std::chrono::steady_clock::now();
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Get or create session
        std::unique_ptr<SessionContext>* session_ptr = nullptr;
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            auto it = sessions_.find(request.session_id);
            if (it == sessions_.end()) {
                sessions_[request.session_id] = std::make_unique<SessionContext>(request.session_id);
                session_ptr = &sessions_[request.session_id];
            } else {
                session_ptr = &it->second;
            }
        }
        
        SessionContext& session = **session_ptr;
        session.update_access_time();
        
        // Execute command based on mode
        std::string result;
        
        if (request.mode == "algebraic" || request.mode.empty()) {
            if (session.algebraic_parser) {
                auto calc_result = session.algebraic_parser->parse(request.command);
                if (calc_result.success) {
                    result = std::to_string(calc_result.value);
                } else {
                    throw std::runtime_error(calc_result.error_message);
                }
            }
        } else if (request.mode == "linear") {
            if (session.linear_parser) {
                auto calc_result = session.linear_parser->parse(request.command);
                if (calc_result.success) {
                    std::ostringstream oss;
                    for (size_t i = 0; i < calc_result.linear_result.solution.size(); ++i) {
                        if (i > 0) oss << ", ";
                        oss << "x" << i << " = " << calc_result.linear_result.solution[i];
                    }
                    result = oss.str();
                } else {
                    throw std::runtime_error(calc_result.error_message);
                }
            }
        }
        
        // Add to session history
        session.history.push_back(request.command + " = " + result);
        
        response.success = true;
        response.result = result;
        
    } catch (const std::exception& e) {
        response.success = false;
        response.error = e.what();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    response.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    return response;
}

void DaemonEngine::update_metrics(double execution_time) {
    // Update running average (simplified exponential moving average)
    double current_avg = avg_response_time_.load();
    double new_avg = current_avg * 0.9 + execution_time * 0.1;
    avg_response_time_.store(new_avg);
}

std::string DaemonEngine::create_session() {
    // Generate unique session ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    
    std::string session_id = "axiom_" + std::to_string(dis(gen));
    
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_[session_id] = std::make_unique<SessionContext>(session_id);
    }
    
    return session_id;
}

bool DaemonEngine::destroy_session(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        sessions_.erase(it);
        return true;
    }
    return false;
}

std::vector<std::string> DaemonEngine::get_active_sessions() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    std::vector<std::string> session_ids;
    session_ids.reserve(sessions_.size());
    
    for (const auto& pair : sessions_) {
        session_ids.push_back(pair.first);
    }
    
    return session_ids;
}

std::chrono::milliseconds DaemonEngine::get_uptime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startup_time_);
}

// ============================================================================
// DaemonClient Implementation
// ============================================================================

DaemonClient::DaemonClient(const std::string& pipe_name)
    : pipe_name_(pipe_name)
    , connected_(false)
#ifdef _WIN32
    , pipe_handle_(INVALID_HANDLE_VALUE)
#else
    , pipe_fd_(-1)
#endif
{
}

DaemonClient::~DaemonClient() {
    disconnect();
}

bool DaemonClient::connect() {
    if (connected_) {
        return true;
    }
    
#ifdef _WIN32
    std::string pipe_path = "\\\\.\\pipe\\" + pipe_name_;
    
    pipe_handle_ = CreateFileA(
        pipe_path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr
    );
    
    connected_ = (pipe_handle_ != INVALID_HANDLE_VALUE);
#else
    std::string pipe_path = "/tmp/" + pipe_name_;
    pipe_fd_ = open(pipe_path.c_str(), O_RDWR);
    connected_ = (pipe_fd_ != -1);
#endif
    
    if (connected_) {
        session_id_ = "client_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    }
    
    return connected_;
}

void DaemonClient::disconnect() {
    if (!connected_) {
        return;
    }
    
#ifdef _WIN32
    if (pipe_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_handle_);
        pipe_handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (pipe_fd_ != -1) {
        close(pipe_fd_);
        pipe_fd_ = -1;
    }
#endif
    
    connected_ = false;
}

DaemonEngine::Response DaemonClient::execute(const std::string& command, const std::string& mode) {
    DaemonEngine::Response response;
    response.success = false;
    
    if (!connected_) {
        response.error = "Not connected to daemon";
        return response;
    }
    
    // Create request (simplified JSON-like format)
    std::ostringstream oss;
    oss << "{\"command\":\"" << command << "\",\"mode\":\"" << mode 
        << "\",\"session\":\"" << session_id_ << "\"}";
    
    std::string request = oss.str();
    
#ifdef _WIN32
    DWORD bytes_written = 0;
    if (WriteFile(pipe_handle_, request.c_str(), request.length(), &bytes_written, nullptr)) {
        // Read response (simplified)
        char buffer[4096];
        DWORD bytes_read = 0;
        if (ReadFile(pipe_handle_, buffer, sizeof(buffer) - 1, &bytes_read, nullptr)) {
            buffer[bytes_read] = '\\0';
            response.result = std::string(buffer);
            response.success = true;
        }
    }
#else
    if (write(pipe_fd_, request.c_str(), request.length()) > 0) {
        // Read response (simplified)
        char buffer[4096];
        ssize_t bytes_read = read(pipe_fd_, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\\0';
            response.result = std::string(buffer);
            response.success = true;
        }
    }
#endif
    
    return response;
}

bool DaemonClient::is_daemon_running(const std::string& pipe_name) {
#ifdef _WIN32
    std::string pipe_path = "\\\\.\\pipe\\" + pipe_name;
    HANDLE handle = CreateFileA(pipe_path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        return true;
    }
    return false;
#else
    std::string pipe_path = "/tmp/" + pipe_name;
    return access(pipe_path.c_str(), F_OK) == 0;
#endif
}

// ============================================================================
// ProcessManager Implementation
// ============================================================================

bool ProcessManager::start_daemon_process(const std::string& pipe_name) {
    // Start daemon as separate process
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    std::string command = "axiom.exe --daemon --pipe=" + pipe_name;
    
    if (CreateProcessA(nullptr, const_cast<char*>(command.c_str()), 
                      nullptr, nullptr, FALSE, CREATE_NO_WINDOW, 
                      nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
#else
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execl("./axiom", "axiom", "--daemon", ("--pipe=" + pipe_name).c_str(), nullptr);
        exit(1);
    }
    return pid > 0;
#endif
}

bool ProcessManager::is_daemon_process_running(const std::string& pipe_name) {
    return DaemonClient::is_daemon_running(pipe_name);
}

} // namespace AXIOM