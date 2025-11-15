#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <functional>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <cmath>
#include <complex>
#include <vector>
#include <string>
#include <stack>
#include <sstream>
#include <queue>
#include <algorithm>

#ifndef CPP_DYNAMIC_CALC_DYNAMIC_CALC_H
#define CPP_DYNAMIC_CALC_DYNAMIC_CALC_H

const double PI_CONST = 3.14159265358979323846;
const double D2R = PI_CONST / 180.0;
const double R2D = 180.0 / PI_CONST;

enum class CalcErr {
    None,
    DivideByZero,
    IndeterminateResult,
    OperationNotFound,
    ArgumentMismatch,
    NegativeRoot,
    DomainError
};

struct OperationResult {
    std::optional<double> result = std::nullopt;
    CalcErr err = CalcErr::None;

    OperationResult() = default;
    OperationResult(std::optional<double> const &res, CalcErr const &error)
            : result(res), err(error) {}
};

enum class Precedence : int {
    None = 0, AddSub = 1, MultiDiv = 2, Pow = 3, Mod = 2, Unary = 4
};

using Operation = std::function<OperationResult(const std::vector<double> &args)>;
using UnaryOperation = std::function<OperationResult(const std::vector<double> &args)>;
struct OperatorDetails {
    Operation operation;
    Precedence precedence;
};
struct UnaryOperatorDetails {
    UnaryOperation operation;
    Precedence precedence = Precedence::Unary;
};

class Dynamic_calc {
private:
    std::map<std::string, OperatorDetails> ops_;
    std::map<std::string, UnaryOperatorDetails> unary_ops_;
    std::shared_mutex mutex_s;

    bool isNumber(const std::string &token) const;
    Precedence get_precedence(const std::string &token) const;
    bool isLeftAssociative(const std::string &token) const;
    bool isSeparator(char c) const;
    std::queue<std::string> ParseToRPN(const std::string &expression);
    OperationResult EvaluateRPN(std::queue<std::string> &rpn_queue);

public:
    Dynamic_calc() {
        std::lock_guard<std::shared_mutex> lock(mutex_s);

        ops_["+"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(args[0] + args[1]), CalcErr::None};
        }, Precedence::AddSub};

        ops_["-"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(args[0] - args[1]), CalcErr::None};
        }, Precedence::AddSub};

        ops_["*"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(args[0] * args[1]), CalcErr::None};
        }, Precedence::MultiDiv};

        ops_["/"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[1] == 0.0) {
                if (args[0] == 0.0) return {std::nullopt, CalcErr::IndeterminateResult};
                return {std::nullopt, CalcErr::DivideByZero};
            }
            return {std::optional<double>(args[0] / args[1]), CalcErr::None};
        }, Precedence::MultiDiv};

        ops_["^"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] == 0.0 && args[1] == 0.0) return {std::nullopt, CalcErr::IndeterminateResult};
            if (args[0] == 0.0 && args[1] < 0.0) return {std::nullopt, CalcErr::DivideByZero};
            return {std::optional<double>(std::pow(args[0], args[1])), CalcErr::None};
        }, Precedence::Pow};

        ops_["%"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[1] == 0.0) {
                if (args[0] == 0.0) return {std::nullopt, CalcErr::IndeterminateResult};
                return {std::nullopt, CalcErr::DivideByZero};
            }
            return {std::optional<double>(std::fmod(args[0], args[1])), CalcErr::None};
        }, Precedence::Mod};

        unary_ops_["sqrt"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] < 0.0) return {std::nullopt, CalcErr::NegativeRoot};
            return {std::optional<double>(std::sqrt(args[0])), CalcErr::None};
        }};

        unary_ops_["exp"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::exp(args[0])), CalcErr::None};
        }};

        unary_ops_["sin"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::sin(args[0] * D2R)), CalcErr::None};
        }};

        unary_ops_["cos"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::cos(args[0] * D2R)), CalcErr::None};
        }};

        unary_ops_["tan"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::tan(args[0] * D2R)), CalcErr::None};
        }};

        unary_ops_["cot"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            double tan_val = std::tan(args[0] * D2R);
            if (std::abs(tan_val) < 1e-9) return {std::nullopt, CalcErr::DivideByZero};
            return {std::optional<double>(1.0 / tan_val), CalcErr::None};
        }};

        unary_ops_["sec"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            double cos_val = std::cos(args[0] * D2R);
            if (std::abs(cos_val) < 1e-9) return {std::nullopt, CalcErr::DivideByZero};
            return {std::optional<double>(1.0 / cos_val), CalcErr::None};
        }};

        unary_ops_["csc"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            double sin_val = std::sin(args[0] * D2R);
            if (std::abs(sin_val) < 1e-9) return {std::nullopt, CalcErr::DivideByZero};
            return {std::optional<double>(1.0 / sin_val), CalcErr::None};
        }};

        unary_ops_["arcsin"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (std::abs(args[0]) > 1.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(std::asin(args[0]) * R2D), CalcErr::None};
        }};

        unary_ops_["arccos"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (std::abs(args[0]) > 1.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(std::acos(args[0]) * R2D), CalcErr::None};
        }};

        unary_ops_["arctan"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::atan(args[0]) * R2D), CalcErr::None};
        }};

        unary_ops_["arccot"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] == 0.0) return {std::optional<double>(90.0), CalcErr::None};
            return {std::optional<double>(std::atan(1.0 / args[0]) * R2D), CalcErr::None};
        }};

        unary_ops_["log"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] <= 0.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(std::log10(args[0])), CalcErr::None};
        }};

        unary_ops_["lg"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] <= 0.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>((std::log(args[0]) / std::log(2.0))), CalcErr::None};
        }};

        unary_ops_["ln"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] <= 0.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(std::log(args[0])), CalcErr::None};
        }};

        unary_ops_["sinh"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::sinh(args[0])), CalcErr::None};
        }};

        unary_ops_["cosh"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::cosh(args[0])), CalcErr::None};
        }};

        unary_ops_["tanh"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::tanh(args[0])), CalcErr::None};
        }};

        unary_ops_["coth"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] == 0.0) return {std::nullopt, CalcErr::DivideByZero};
            return {std::optional<double>(1.0 / std::tanh(args[0])), CalcErr::None};
        }};

        unary_ops_["sech"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(1.0 / std::cosh(args[0])), CalcErr::None};
        }};

        unary_ops_["csch"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] == 0.0) return {std::nullopt, CalcErr::DivideByZero};
            return {std::optional<double>(1.0 / std::sinh(args[0])), CalcErr::None};
        }};

        unary_ops_["arcsinh"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            return {std::optional<double>(std::asinh(args[0])), CalcErr::None};
        }};

        unary_ops_["arccosh"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (args[0] < 1.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(std::acosh(args[0])), CalcErr::None};
        }};

        unary_ops_["arctanh"] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
            if (std::abs(args[0]) >= 1.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(std::atanh(args[0])), CalcErr::None};
        }};

        unary_ops_["arccoth"] = {[](const std::vector<double>& args)->OperationResult{
            if(args.size()!=1) return {std::nullopt,CalcErr::ArgumentMismatch};
            if(std::abs(args[0]) <= 1.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(0.5 * std::log((args[0]+1.0)/(args[0]-1.0))), CalcErr::None};
        }};

        unary_ops_["arcsech"] = {[](const std::vector<double>& args)->OperationResult{
            if(args.size()!=1) return {std::nullopt,CalcErr::ArgumentMismatch};
            if(args[0] <= 0.0 || args[0] > 1.0) return {std::nullopt, CalcErr::DomainError};
            return {std::optional<double>(std::acosh(1.0/args[0])), CalcErr::None};
        }};

        unary_ops_["arccsch"] = {[](const std::vector<double>& args)->OperationResult{
            if(args.size()!=1) return {std::nullopt,CalcErr::ArgumentMismatch};
            if(args[0] == 0.0) return {std::nullopt, CalcErr::DivideByZero};
            return {std::optional<double>(std::asinh(1.0/args[0])), CalcErr::None};
        }};
    }

    OperationResult Evaluate(const std::string &expression);
    void RegisterOperator(std::string op, const OperatorDetails &details);
    void RegisterUnaryOperator(std::string op, const UnaryOperatorDetails &details);

    Dynamic_calc(const Dynamic_calc &other) = delete;
    Dynamic_calc(Dynamic_calc &&other) noexcept = delete;
    ~Dynamic_calc() noexcept = default;
};

#endif //CPP_DYNAMIC_CALC_DYNAMIC_CALC_H