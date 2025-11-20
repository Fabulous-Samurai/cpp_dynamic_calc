#include "algebraic_parser.h"
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <stack>
#include <iostream>
#include <limits>

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
        if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] == 0.0 && args[1] == 0.0) return MAKE_ERROR(CalcErr::IndeterminateResult);
        if (args[0] == 0.0 && args[1] < 0.0) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(std::pow(args[0], args[1]));
    }, Precedence::Pow};

    ops_["%"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[1] == 0.0) {
            if (args[0] == 0.0) return MAKE_ERROR(CalcErr::IndeterminateResult);
            return MAKE_ERROR(CalcErr::DivideByZero);
        }
        return MAKE_SUCCESS(std::fmod(args[0], args[1]));
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
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::sin(args[0] * D2R)); 
    }};

    unary_ops_["cos"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::cos(args[0] * D2R)); 
    }};

    unary_ops_["tan"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::tan(args[0] * D2R)); 
    }};

    unary_ops_["cot"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        double tan_val = std::tan(args[0] * D2R);
        if (std::abs(tan_val) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(1.0 / tan_val);
    }};

    unary_ops_["sec"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        double cos_val = std::cos(args[0] * D2R);
        if (std::abs(cos_val) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(1.0 / cos_val);
    }};

    unary_ops_["csc"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        double sin_val = std::sin(args[0] * D2R);
        if (std::abs(sin_val) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(1.0 / sin_val);
    }};

    unary_ops_["arcsin"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (std::abs(args[0]) > 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::asin(args[0]) * R2D);
    }};

    unary_ops_["arccos"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (std::abs(args[0]) > 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::acos(args[0]) * R2D);
    }};

    unary_ops_["arctan"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::atan(args[0]) * R2D);
    }};

    unary_ops_["arccot"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] == 0.0) return MAKE_SUCCESS(90.0);
        return MAKE_SUCCESS(std::atan(1.0 / args[0]) * R2D);
    }};

    unary_ops_["arcsec"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        double x = args[0];
        if (std::abs(x) < 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::acos(1.0 / x) * R2D);
    }};

    unary_ops_["arccsc"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        double x = args[0];
        if (std::abs(x) < 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::asin(1.0 / x) * R2D);
    }};

    unary_ops_["log"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] <= 0.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::log10(args[0]));
    }};

    unary_ops_["lg"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] <= 0.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::log(args[0]) / std::log(2.0));
    }};

    unary_ops_["ln"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] <= 0.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::log(args[0]));
    }};

    unary_ops_["sinh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::sinh(args[0]));
    }};

    unary_ops_["cosh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::cosh(args[0]));
    }};

    unary_ops_["tanh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::tanh(args[0]));
    }};

    unary_ops_["coth"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] == 0.0) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(1.0 / std::tanh(args[0]));
    }};

    unary_ops_["sech"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(1.0 / std::cosh(args[0]));
    }};

    unary_ops_["csch"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] == 0.0) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(1.0 / std::sinh(args[0]));
    }};

    unary_ops_["arcsinh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        return MAKE_SUCCESS(std::asinh(args[0]));
    }};

    unary_ops_["arccosh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] < 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::acosh(args[0]));
    }};

    unary_ops_["arctanh"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (std::abs(args[0]) >= 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::atanh(args[0]));
    }};

    unary_ops_["arccoth"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (std::abs(args[0]) <= 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(0.5 * std::log((args[0] + 1.0) / (args[0] - 1.0)));
    }};

    unary_ops_["arcsech"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] <= 0.0 || args[0] > 1.0) return MAKE_ERROR(CalcErr::DomainError);
        return MAKE_SUCCESS(std::acosh(1.0 / args[0]));
    }};

    unary_ops_["arccsch"] = {[](const std::vector<double> &args) -> EngineResult {
        if (args.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
        if (args[0] == 0.0) return MAKE_ERROR(CalcErr::DivideByZero);
        return MAKE_SUCCESS(std::asinh(1.0 / args[0]));
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
            // Eğer bir fonksiyon varsa, onu da kuyruğa ekleyin
            if (!operator_stack.empty() && unary_ops_.count(operator_stack.top())) {
                output_queue.push(operator_stack.top());
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
    std::cout << "Evaluating RPN queue." << std::endl;
    std::stack<double> value_stack;
    std::shared_lock<std::shared_mutex> lock(mutex_s);

    while (!rpn_queue.empty()) {
        std::string token = rpn_queue.front();
        rpn_queue.pop();

        std::cout << "Processing token: " << token << std::endl;

        if (isNumber(token)) {
            double value = std::stod(token);
            std::cout << "Pushing number onto stack: " << value << std::endl;
            value_stack.push(value);
        } else if (ops_.count(token)) {
            if (value_stack.size() < 2) {
                std::cout << "Error: Not enough values on stack for binary operator: " << token << std::endl;
                return MAKE_ERROR(CalcErr::ArgumentMismatch);
            }

            double val2 = value_stack.top();
            value_stack.pop();
            double val1 = value_stack.top();
            value_stack.pop();

            std::cout << "Applying operator " << token << " to values: " << val1 << ", " << val2 << std::endl;

            std::vector<double> args = {val1, val2};
            EngineResult op_res = ops_.at(token).operation(args);

            if (op_res.error.has_value()) {
                std::cout << "Error during operation: " << token << std::endl;
                return op_res;
            }

            double result = std::get<double>(op_res.result.value());
            std::cout << "Pushing result onto stack: " << result << std::endl;
            value_stack.push(result);
        } else if (unary_ops_.count(token)) {
            if (value_stack.empty()) {
                std::cout << "Error: Not enough values on stack for unary operator: " << token << std::endl;
                return MAKE_ERROR(CalcErr::ArgumentMismatch);
            }

            double val = value_stack.top();
            value_stack.pop();

            std::cout << "Applying unary operator " << token << " to value: " << val << std::endl;

            std::vector<double> args = {val};
            EngineResult op_res = unary_ops_.at(token).operation(args);

            if (op_res.error.has_value()) {
                std::cout << "Error during unary operation: " << token << std::endl;
                return op_res;
            }

            double result = std::get<double>(op_res.result.value());
            std::cout << "Pushing result onto stack: " << result << std::endl;
            value_stack.push(result);
        }
    }

    if (value_stack.size() != 1) {
        std::cout << "Error: Stack does not contain exactly one value after evaluation." << std::endl;
        return MAKE_ERROR(CalcErr::ArgumentMismatch);
    }

    double final_result = value_stack.top();
    std::cout << "Final result: " << final_result << std::endl;
    return MAKE_SUCCESS(final_result);
}