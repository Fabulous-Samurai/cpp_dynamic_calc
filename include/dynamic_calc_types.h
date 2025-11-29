#pragma once

#include <vector>
#include <optional>
#include <variant>
#include <functional>
#include <string>
#include <cmath>
#include <limits>

enum class CalcErr
{
    None,
    DivideByZero,
    IndeterminateResult,
    OperationNotFound,
    ArgumentMismatch,
    NegativeRoot,
    DomainError,
    ParseError,
    // New stability errors
    NumericOverflow,
    StackOverflow,
    MemoryExhausted,
    InfiniteLoop
};

enum class LinAlgErr
{
    None,
    NoSolution,
    InfiniteSolutions,
    MatrixMismatch,
    ParseError
};
using Matrix = std::vector<std::vector<double>>;
using Vector = std::vector<double>;
using EngineSuccessResult = std::variant<double, Vector, Matrix,std::string>;
using EngineErrorResult = std::variant<CalcErr, LinAlgErr>;

struct EngineResult
{
    std::optional<EngineSuccessResult> result = std::nullopt;
    std::optional<EngineErrorResult> error = std::nullopt;
};

const double PI_CONST = 3.14159265358979323846;
const double D2R = PI_CONST / 180.0;
const double R2D = 180.0 / PI_CONST;

#include <cmath>

// Safe arithmetic operations
namespace SafeMath {
    constexpr double MAX_SAFE_DOUBLE = 1e100;
    constexpr double MIN_SAFE_DOUBLE = -1e100;
    
    inline bool IsFiniteAndSafe(double val) {
        return std::isfinite(val) && val <= MAX_SAFE_DOUBLE && val >= MIN_SAFE_DOUBLE;
    }
    
    inline std::optional<double> SafeAdd(double a, double b) {
        if (!IsFiniteAndSafe(a) || !IsFiniteAndSafe(b)) return std::nullopt;
        double result = a + b;
        return IsFiniteAndSafe(result) ? std::optional<double>(result) : std::nullopt;
    }
    
    inline std::optional<double> SafePow(double base, double exp) {
        if (!IsFiniteAndSafe(base) || !IsFiniteAndSafe(exp)) return std::nullopt;
        if (std::abs(exp) > 100 || std::abs(base) > 1e10) return std::nullopt; // Prevent huge exponents
        double result = std::pow(base, exp);
        return IsFiniteAndSafe(result) ? std::optional<double>(result) : std::nullopt;
    }
}

enum class Precedence : int
{
    None = 0,
    AddSub = 1,
    MultiDiv = 2,
    Pow = 3,
    Mod = 2,
    Unary = 4
};

using Operation = std::function<EngineResult(const std::vector<double> &args)>;
using UnaryOperation = std::function<EngineResult(const std::vector<double> &args)>;

struct OperatorDetails
{
    Operation operation;
    Precedence precedence;
};

struct UnaryOperatorDetails
{
    UnaryOperation operation;
    Precedence precedence = Precedence::Unary;
};
