#pragma once

#include <vector>
#include <optional>
#include <variant>
#include <functional>
#include <string>

enum class CalcErr
{
    None,
    DivideByZero,
    IndeterminateResult,
    OperationNotFound,
    ArgumentMismatch,
    NegativeRoot,
    DomainError
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
using EngineSuccessResult = std::variant<double, Vector, Matrix>;
using EngineErrorResult = std::variant<CalcErr, LinAlgErr>;

struct EngineResult
{
    std::optional<EngineSuccessResult> result = std::nullopt;
    std::optional<EngineErrorResult> error = std::nullopt;
};

const double PI_CONST = 3.14159265358979323846;
const double D2R = PI_CONST / 180.0;
const double R2D = 180.0 / PI_CONST;

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
