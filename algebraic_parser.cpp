#include "algebraic_parser.h"
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <stack>
#include <iostream>

#define MAKE_ERROR(error_code) {{},EngineErrorResult(error_code)}
#define MAKE_SUCCESS(value) {EngineSuccessResult(value),{}}

AlgebraicParser::AlgebraicParser() {
    std::lock_guard<std::shared_mutex> lock(mutex_s);

    ops_["+"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(args[0] + args[1]);
    }, Precedence::AddSub};

    ops_["-"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(args[0] - args[1]);
    }, Precedence::AddSub};

    ops_["*"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(args[0] * args[1]);
    }, Precedence::MultiDiv};

    ops_["/"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[1] == 0.0) {
            if (args[0] == 0.0) return MAKE_ERROR(CalcErr::IndeterminateResult);
            return MAKE_ERROR(CalcErr::DivideByZero);
        }
        return MAKE_SUCCESS(args[0] / args[1]);
    }, Precedence::MultiDiv};

    ops_["^"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] == 0.0 && args[1] == 0.0) return MAKE_ERROR(CalcErr::IndeterminateResult);
        if (args[0] == 0.0 && args[1] < 0.0) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(std::pow(args[0],args[1]));
    }, Precedence::Pow};

    ops_["%"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 2) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[1] == 0.0) {
            if (args[0] == 0.0) return {std::nullopt, CalcErr::IndeterminateResult};
            return {std::nullopt, CalcErr::DivideByZero};
        }
        return {std::optional<double>(std::fmod(args[0], args[1])), CalcErr::None};
    }, Precedence::Mod};

    unary_ops_["sqrt"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] < 0.0) return {std::nullopt, CalcErr::NegativeRoot};
        return {std::optional<double>(std::sqrt(args[0])), CalcErr::None};
    }};

    unary_ops_["exp"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::exp(args[0])), CalcErr::None};
    }};

    unary_ops_["sin"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::sin(args[0] * D2R)), CalcErr::None};
    }};

    unary_ops_["cos"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::cos(args[0] * D2R)), CalcErr::None};
    }};

    unary_ops_["tan"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::tan(args[0] * D2R)), CalcErr::None};
    }};

    unary_ops_["cot"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        double tan_val = std::tan(args[0] * D2R);
        if (std::abs(tan_val) < 1e-9) return {std::nullopt, CalcErr::DivideByZero};
        return {std::optional<double>(1.0 / tan_val), CalcErr::None};
    }};

    unary_ops_["sec"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        double cos_val = std::cos(args[0] * D2R);
        if (std::abs(cos_val) < 1e-9) return {std::nullopt, CalcErr::DivideByZero};
        return {std::optional<double>(1.0 / cos_val), CalcErr::None};
    }};

    unary_ops_["csc"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        double sin_val = std::sin(args[0] * D2R);
        if (std::abs(sin_val) < 1e-9) return {std::nullopt, CalcErr::DivideByZero};
        return {std::optional<double>(1.0 / sin_val), CalcErr::None};
    }};

    unary_ops_["arcsin"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (std::abs(args[0]) > 1.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::asin(args[0]) * R2D), CalcErr::None};
    }};

    unary_ops_["arccos"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (std::abs(args[0]) > 1.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::acos(args[0]) * R2D), CalcErr::None};
    }};

    unary_ops_["arctan"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::atan(args[0]) * R2D), CalcErr::None};
    }};

    unary_ops_["arccot"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] == 0.0) return {std::optional<double>(90.0), CalcErr::None};
        return {std::optional<double>(std::atan(1.0 / args[0]) * R2D), CalcErr::None};
    }};

    unary_ops_["arcsec"]={[](const std::vector<double>&args)->EngineResult{
        if(args.size()!=1) return {std::nullopt,CalcErr::ArgumentMismatch};
        double x = args[0];
        if(std::abs(x) < 1.0) return {std::nullopt,CalcErr::DomainError};
        return {std::optional<double>(std::acos(1.0 / x)*R2D),CalcErr::None};
    }};

    unary_ops_["arccsc"] = {[](const std::vector<double>&args)->EngineResult{
        if(args.size()!=1) return {std::nullopt , CalcErr::ArgumentMismatch};
        double x  = args[0];
        if(std::abs(x) < 1.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::asin(1.0 / x)*R2D),CalcErr::None};
    }};

    unary_ops_["log"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] <= 0.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::log10(args[0])), CalcErr::None};
    }};

    unary_ops_["lg"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] <= 0.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>((std::log(args[0]) / std::log(2.0))), CalcErr::None};
    }};

    unary_ops_["ln"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] <= 0.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::log(args[0])), CalcErr::None};
    }};

    unary_ops_["sinh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::sinh(args[0])), CalcErr::None};
    }};

    unary_ops_["cosh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::cosh(args[0])), CalcErr::None};
    }};

    unary_ops_["tanh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::tanh(args[0])), CalcErr::None};
    }};

    unary_ops_["coth"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] == 0.0) return {std::nullopt, CalcErr::DivideByZero};
        return {std::optional<double>(1.0 / std::tanh(args[0])), CalcErr::None};
    }};

    unary_ops_["sech"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(1.0 / std::cosh(args[0])), CalcErr::None};
    }};

    unary_ops_["csch"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] == 0.0) return {std::nullopt, CalcErr::DivideByZero};
        return {std::optional<double>(1.0 / std::sinh(args[0])), CalcErr::None};
    }};

    unary_ops_["arcsinh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        return {std::optional<double>(std::asinh(args[0])), CalcErr::None};
    }};

    unary_ops_["arccosh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (args[0] < 1.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::acosh(args[0])), CalcErr::None};
    }};

    unary_ops_["arctanh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return {std::nullopt, CalcErr::ArgumentMismatch};
        if (std::abs(args[0]) >= 1.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::atanh(args[0])), CalcErr::None};
    }};

    unary_ops_["arccoth"] = {[](const std::vector<double>& args)->EngineResult{
        if(args.size()!=1) return {std::nullopt,CalcErr::ArgumentMismatch};
        if(std::abs(args[0]) <= 1.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(0.5 * std::log((args[0]+1.0)/(args[0]-1.0))), CalcErr::None};
    }};

    unary_ops_["arcsech"] = {[](const std::vector<double>& args)->EngineResult{
        if(args.size()!=1) return {std::nullopt,CalcErr::ArgumentMismatch};
        if(args[0] <= 0.0 || args[0] > 1.0) return {std::nullopt, CalcErr::DomainError};
        return {std::optional<double>(std::acosh(1.0/args[0])), CalcErr::None};
    }};

    unary_ops_["arccsch"] = {[](const std::vector<double>& args)->EngineResult{
        if(args.size()!=1) return {std::nullopt,CalcErr::ArgumentMismatch};
        if(args[0] == 0.0) return {std::nullopt, CalcErr::DivideByZero};
        return {std::optional<double>(std::asinh(1.0/args[0])), CalcErr::None};
    }};
}

void AlgebraicParser::RegisterOperator(const std::string &op, const OperatorDetails &details) {
    std::lock_guard<std::shared_mutex> lock(mutex_s);
    ops_[op] = details;
}

void AlgebraicParser::RegisterUnaryOperator(const std::string &op, const UnaryOperatorDetails &details) {
    std::lock_guard<std::shared_mutex> lock(mutex_s);
    unary_ops_[op] = details;
}

EngineResult AlgebraicParser::ParseAndExecute(const std::string& input) {
    std::queue<std::string> rpn = ParseToRPN(input);
    EngineResult op_res = EvaluateRPN(rpn);

    if (!op_res.error.has_value()) {
        return {op_res.result.value(), {}};
    } else {
        return {{}, {op_res.error}};
    }
}

bool AlgebraicParser::isNumber(const std::string &token) const {
    if (token.empty()) return false;
    char *end = nullptr;
    std::strtod(token.c_str(), &end);
    return (end == (token.c_str() + token.size()));
}

Precedence AlgebraicParser::get_precedence(const std::string &token) const {
    std::shared_lock<std::shared_mutex> lock(mutex_s);
    if (ops_.count(token)) {
        return ops_.at(token).precedence;
    }
    if (unary_ops_.count(token)) {
        return unary_ops_.at(token).precedence;
    }
    return Precedence::None;
}

bool AlgebraicParser::isSeparator(char c) const {
    std::string s(1, c);
    std::shared_lock<std::shared_mutex> lock(mutex_s);
    return (c == '(' || c == ')') || ops_.count(s);
}

bool AlgebraicParser::isLeftAssociative(const std::string &token) const {
    if (token == "^") {
        return false;
    }
    return true;
}

std::queue<std::string> AlgebraicParser::ParseToRPN(const std::string &expression) {
    std::queue<std::string> output_queue;
    std::stack<std::string> operator_stack;

    std::string prepared_expression;
    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];
        if (isspace(c)) {
            prepared_expression += ' ';
        } else if (isSeparator(c)) {
            prepared_expression += ' ';
            prepared_expression += c;
            prepared_expression += ' ';
        } else {
            prepared_expression += c;
        }
    }

    std::stringstream ss(prepared_expression);
    std::string token;

    while (ss >> token) {
        if (isNumber(token)) {
            output_queue.push(token);
        } else if (unary_ops_.count(token)) {
            operator_stack.push(token);
        } else if (ops_.count(token)) {
            Precedence current_precedence = get_precedence(token);
            while (!operator_stack.empty() &&
                   operator_stack.top() != "(" &&
                   (get_precedence(operator_stack.top()) > current_precedence ||
                    (get_precedence(operator_stack.top()) == current_precedence && isLeftAssociative(token)))
                    ) {
                output_queue.push(operator_stack.top());
                operator_stack.pop();
            }
            operator_stack.push(token);
        } else if (token == "(") {
            operator_stack.push(token);
        } else if (token == ")") {
            while (!operator_stack.empty() && operator_stack.top() != "(") {
                output_queue.push(operator_stack.top());
                operator_stack.pop();
            }
            if (!operator_stack.empty()) {
                operator_stack.pop();
            }
        }
    }

    while (!operator_stack.empty()) {
        output_queue.push(operator_stack.top());
        operator_stack.pop();
    }
    return output_queue;
}

EngineResult AlgebraicParser::EvaluateRPN(std::queue<std::string> &rpn_queue) {
    std::stack<double> value_stack;
    std::shared_lock<std::shared_mutex> lock(mutex_s);

    while (!rpn_queue.empty()) {
        std::string token = rpn_queue.front();
        rpn_queue.pop();

        if (isNumber(token)) {
            value_stack.push(std::stod(token));
        } else if (ops_.count(token)) {
            if (value_stack.size() < 2) {
                return MAKE_ERROR(CalcErr::ArgumentMismatch);
            }

            double val2 = value_stack.top();
            value_stack.pop();
            double val1 = value_stack.top();
            value_stack.pop();

            std::vector<double> args = {val1, val2};
            EngineResult op_res = ops_.at(token).operation(args);

            if (op_res.error.has_value()) {
                return op_res;
            }

            value_stack.push(std::get<double>(op_res.result.value()));
        } else if (unary_ops_.count(token)) {
            if (value_stack.empty()) {
                return MAKE_ERROR(CalcErr::ArgumentMismatch);
            }

            double val = value_stack.top();
            value_stack.pop();

            std::vector<double> args = {val};
            EngineResult op_res = unary_ops_.at(token).operation(args);

            if (op_res.error.has_value()) {
                return op_res;
            }

            value_stack.push(std::get<double>(op_res.result.value()));
        }
    }

    if (value_stack.size() != 1) {
        return MAKE_ERROR(CalcErr::ArgumentMismatch);
    }

    return MAKE_SUCCESS(value_stack.top());
}