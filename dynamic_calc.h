//
// Created by Efe on 14.11.2025.
//
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


#ifndef CPP_DYNAMIC_CALC_DYNAMIC_CALC_H
#define CPP_DYNAMIC_CALC_DYNAMIC_CALC_H


enum class CalcErr {
    None,
    DivideByZero,
    IndeterminateResult,
    OperationNotFound,
    ArgumentMismatch,
    NegativeRoot,
    LeftAssociative
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
    std::map<char, OperatorDetails> ops_;
    std::map<char, UnaryOperatorDetails> unary_ops_;
    std::shared_mutex mutex_s;

    bool isNumber(const std::string &token) const;
    Precedence get_precedence(const std::string &token) const;
    bool isLeftAssociative(const std::string &token) const;
    bool isSeperator(char c)const;

    std::queue<std::string> ParseToRPN(const std::string &expression);

    OperationResult EvaluateRPN(std::queue<std::string> &rpn_queue);

public:
    Dynamic_calc() {
        std::lock_guard<std::shared_mutex> lock(mutex_s);
        ops_['+'] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) {
                std::cerr << "UB for definition of essential algebraic operator!";
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            return OperationResult{std::optional<double>(args[0] + args[1]), CalcErr::None};

        }, Precedence::AddSub
        };
        ops_['-'] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) {
                std::cerr << "UB acc to definition of essential algebraic operator!";
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            return OperationResult{std::optional<double>(args[0] - args[1]), CalcErr::None};

        }, Precedence::AddSub
        };
        ops_['*'] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) {
                std::cerr << "UB acc to definition of essential algebraic operator!";
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            return OperationResult{std::optional<double>(args[0] * args[1]), CalcErr::None};

        }, Precedence::MultiDiv
        };
        ops_['/'] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) {
                std::cerr << "UB acc to definition of essential algebraic operator!";
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            if (args[1] == 0.0) {
                if (args[0] == 0.0) {
                    return OperationResult{std::nullopt, CalcErr::IndeterminateResult};
                }
                return OperationResult{std::nullopt, CalcErr::DivideByZero};
            }

            return OperationResult{std::optional<double>(args[0] / args[1]), CalcErr::None};

        }, Precedence::MultiDiv};
        ops_['^'] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) {
                std::cerr << "UB acc to definition of essential algebraic operator!";
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            if (args[0] == 0.0 && args[1] == 0.0) {
                return OperationResult{std::nullopt, CalcErr::IndeterminateResult};
            }

            if (args[0] == 0.0 && args[1] < 0.0) {
                return OperationResult{std::nullopt, CalcErr::DivideByZero};
            }

            return OperationResult{std::optional<double>(std::pow(args[0], args[1])), CalcErr::None};
        }, Precedence::Pow
        };
        ops_['%'] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 2) {
                std::cerr << "UB acc to definition of essential algebraic operator!";
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            if (args[1] == 0.0) {
                if (args[0] == 0.0) { // 0 mod 0
                    return OperationResult{std::nullopt, CalcErr::IndeterminateResult};
                }
                return OperationResult{std::nullopt, CalcErr::DivideByZero}; // N mod 0
            }

            return OperationResult{std::optional<double>(std::fmod(args[0], args[1])), CalcErr::None};
        },   Precedence::Mod
        };
        unary_ops_['s'] = {[](const std::vector<double> &args) -> OperationResult {
            if (args.size() != 1) {
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            double x = args[0];
            if (x < 0.0) {
                return {std::nullopt, CalcErr::NegativeRoot};
            }
            return {std::optional<double>(std::sqrt(x)), CalcErr::None};
        }};
    }

    OperationResult Evaluate(const std::string &expression) {
        std::queue<std::string> rpn = ParseToRPN(expression);

        return EvaluateRPN(rpn);
    };

    Dynamic_calc(const Dynamic_calc &other) = delete;

    Dynamic_calc(Dynamic_calc &&other) noexcept = delete;

    ~Dynamic_calc() noexcept = default;
};

#endif //CPP_DYNAMIC_CALC_DYNAMIC_CALC_H
