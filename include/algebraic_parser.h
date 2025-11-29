#pragma once
#include "IParser.h"
#include "dynamic_calc_types.h" // Contains 'enum class Precedence'
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <cstring>
#include <cmath>
#include <charconv>
#include <optional>

// ========================================================
// 1. MEMORY ARENA (High Performance Allocation)
// ========================================================
class Arena {
    struct Block { char* memory; size_t size; size_t used; };
    std::vector<Block> blocks;
public:
    Arena(size_t blockSize = 1024 * 64) { allocateBlock(blockSize); }
    ~Arena() { for (auto& block : blocks) delete[] block.memory; }
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    
    void allocateBlock(size_t size) { char* mem = new char[size]; blocks.push_back({mem, size, 0}); }
    void reset() { for (auto& block : blocks) delete[] block.memory; blocks.clear(); allocateBlock(1024 * 64); }
    
    template <typename T, typename... Args>
    T* alloc(Args&&... args) {
        size_t sizeNeeded = sizeof(T); size_t align = alignof(T);
        Block* current = &blocks.back();
        uintptr_t currentPtr = (uintptr_t)(current->memory + current->used);
        size_t padding = (align - (currentPtr % align)) % align;
        
        if (current->used + padding + sizeNeeded > current->size) {
            allocateBlock(std::max(current->size * 2, sizeNeeded + align));
            current = &blocks.back(); currentPtr = (uintptr_t)(current->memory); padding = 0;
        }
        current->used += padding; void* ptr = current->memory + current->used; current->used += sizeNeeded;
        return new (ptr) T(std::forward<Args>(args)...);
    }
    
    std::string_view allocString(std::string_view sv) {
        size_t len = sv.length(); Block* current = &blocks.back();
        if (current->used + len > current->size) { allocateBlock(std::max(current->size * 2, len)); current = &blocks.back(); }
        char* ptr = current->memory + current->used; std::memcpy(ptr, sv.data(), len); current->used += len;
        return std::string_view(ptr, len);
    }
};

// ========================================================
// 2. AST NODE DEFINITIONS
// ========================================================

struct ExprNode;
using NodePtr = ExprNode*;

struct ExprNode {
    virtual ~ExprNode() = default;
    virtual double Evaluate(const std::map<std::string, double>& vars) const = 0;
    virtual NodePtr Derivative(Arena& arena, std::string_view var) const = 0;
    virtual NodePtr Simplify(Arena& arena) const = 0;

    // [UPDATED] Smart Pretty Printer
    // Uses 'Precedence' enum to decide if parentheses are needed.
    // Default is None (lowest precedence), meaning no parentheses.
    virtual std::string ToString(Precedence parent_prec = Precedence::None) const = 0;
};

// ========================================================
// 3. PARSER CLASS DEFINITION
// ========================================================

class AlgebraicParser : public IParser {
public:
    AlgebraicParser();
    
    // Standard execution
    EngineResult ParseAndExecute(const std::string& input) override;
    
    // [NEW] Execution with Context (Critical for 'Ans' variable)
    EngineResult ParseAndExecuteWithContext(const std::string& input, const std::map<std::string, double>& context);

private:
    Arena arena_;
    mutable std::shared_mutex mutex_s;

    struct CommandEntry { std::string command; std::function<EngineResult(const std::string&)> handler; };
    std::vector<CommandEntry> special_commands_;

    void RegisterSpecialCommands();
    NodePtr ParseExpression(std::string_view input);
    
    EngineResult HandleQuadratic(const std::string& input);
    EngineResult HandleNonLinearSolve(const std::string& input);
    EngineResult HandleDerivative(const std::string& input);
    
    EngineResult SolveQuadratic(double a, double b, double c);
    EngineResult SolveNonLinearSystem(const std::vector<std::string>& equations, std::map<std::string, double>& guess);
};