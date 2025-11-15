#include "dynamic_calc.h"
#include <stack>
#include <stdexcept>
#include <sstream>
#include <iostream>

bool Dynamic_calc::isNumber(const std::string &token) const {
    if (token.empty()) return false;
    char *end = nullptr;
    std::strtod(token.c_str(), &end);
    return (end == (token.c_str() + token.size()));
}

Precedence Dynamic_calc::get_precedence(const std::string &token) const {
    if (ops_.count(token)) {
        return ops_.at(token).precedence;
    }
    if (unary_ops_.count(token)) {
        return unary_ops_.at(token).precedence;
    }
    return Precedence::None;
}

bool Dynamic_calc::isSeparator(char c) const {
    std::string s(1, c);
    return (c == '(' || c == ')') || ops_.count(s);
}

bool Dynamic_calc::isLeftAssociative(const std::string &token) const {
    if (token == "^") {
        return false;
    }
    return true;
}

std::queue<std::string> Dynamic_calc::ParseToRPN(const std::string &expression) {
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

OperationResult Dynamic_calc::EvaluateRPN(std::queue<std::string> &rpn_queue) {
    std::stack<double> value_stack;
    while (!rpn_queue.empty()) {
        std::string token = rpn_queue.front();
        rpn_queue.pop();

        if (isNumber(token)) {
            try {
                value_stack.push(std::stod(token));
            } catch (std::exception &e) {
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
        } else if (ops_.count(token)) {
            if (value_stack.size() < 2) {
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            double val2 = value_stack.top();
            value_stack.pop();
            double val1 = value_stack.top();
            value_stack.pop();

            std::vector<double> args = {val1, val2};
            OperationResult op_res = ops_.at(token).operation(args);
            if (op_res.err != CalcErr::None) {
                return op_res;
            }
            value_stack.push(op_res.result.value());
        } else if (unary_ops_.count(token)) {
            if (value_stack.size() < 1) {
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            double val = value_stack.top();
            value_stack.pop();
            std::vector<double> args = {val};
            OperationResult unary_op_res = unary_ops_.at(token).operation(args);
            if (unary_op_res.err != CalcErr::None) {
                return unary_op_res;
            }
            value_stack.push(unary_op_res.result.value());
        }
    }

    if (value_stack.size() != 1) {
        return {std::nullopt, CalcErr::ArgumentMismatch};
    }

    return {std::optional<double>(value_stack.top()), CalcErr::None};
}

OperationResult Dynamic_calc::Evaluate(const std::string &expression) {
    std::queue<std::string> rpn = ParseToRPN(expression);
    return EvaluateRPN(rpn);
}

void Dynamic_calc::RegisterOperator(std::string op, const OperatorDetails &details) {
    std::lock_guard<std::shared_mutex> lock(mutex_s);
    ops_[op] = details;
}

void Dynamic_calc::RegisterUnaryOperator(std::string op, const UnaryOperatorDetails &details) {
    std::lock_guard<std::shared_mutex> lock(mutex_s);
    unary_ops_[op] = details;
}