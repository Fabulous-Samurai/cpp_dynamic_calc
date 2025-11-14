#include <iostream>
#include "dynamic_calc.h"
#include <stack>
#include <stdexcept>
#include <sstream>

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
        } else if (ops_.count(token[0])) {
            if (value_stack.size() < 2) {
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            double val2 = value_stack.top();
            value_stack.pop();
            double val1 = value_stack.top();
            value_stack.pop();

            std::vector<double> args = {val1, val2};
            OperationResult op_res = ops_.at(token[0]).operation(args);
            if (op_res.err != CalcErr::None) {
                return op_res;
            }
            value_stack.push(op_res.result.value());
        } else if (unary_ops_.count(token[0])) {
            if (value_stack.size() != 1) {
                return {std::nullopt, CalcErr::ArgumentMismatch};
            }
            double val = value_stack.top();
            value_stack.pop();
            std::vector<double> args = {val};
            OperationResult unary_op_res = unary_ops_.at(token[0]).operation(args);
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
};

bool Dynamic_calc::isNumber(const std::string &token) const {
    if (token.empty()) return false;
    char *end = nullptr;
    std::strtod(token.c_str(), &end);
    return (end == (token.c_str() + token.size()));
};

Precedence Dynamic_calc::get_precedence(const std::string &token) const {
    if (token.length() != 1) return Precedence::None;
    char op = token[0];
    if (ops_.count(op)) {
        return ops_.at(op).precedence;
    }
    if (unary_ops_.count(op)) {
        return unary_ops_.at(op).precedence;
    }
    return Precedence::None;
};

bool Dynamic_calc::isLeftAssociative(const std::string &token) const {
    if (token == "^") {
        return false;
    }
    return true;
};


bool Dynamic_calc::isSeperator(char c) const {

    return ops_.count(c) || unary_ops_.count(c) || c== '(' || c == ')';
};

std::queue<std::string> Dynamic_calc::ParseToRPN(const std::string &expression) {
    std::queue<std::string> output_queue;
    std::stack<std::string> operator_stack;

    std::string prepared_expression;
    for(size_t i = 0; i <expression.length();i++){
        char c = expression[i];

        if(isspace(c)){
            prepared_expression += ' ';
        }
        else if (isSeperator(c)){
            prepared_expression += ' ';
            prepared_expression += c;
            prepared_expression += ' ';
        }else {
            prepared_expression += c;
        }
    }

    std::stringstream ss(prepared_expression);
    std::string  token;
    while (ss >> token) {

        if (isNumber(token)) {
            output_queue.push(token);
        } else if (token.length() == 1 && unary_ops_.count(token[0])) {
            operator_stack.push(token);
        } else if (token.length() == 1 && ops_.count(token[0])) {
            Precedence current_precedence = ops_.at(token[0]).precedence;


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
};

int main() {
    std::string expression;
    Dynamic_calc calc_;
    std::cout << "Obtain an Expression\n";
    std::getline(std::cin, expression);
    auto evaluate_result = calc_.Evaluate(expression);
    if (evaluate_result.err == CalcErr::None) {
        std::cout << "Result :" << evaluate_result.result.value() << "\n";
    } else {
        std::cerr << "Error:";
        switch (evaluate_result.err) {
            case CalcErr::DivideByZero :
                std::cerr << "Can't Dividable by 0!";
                break;
            case CalcErr::IndeterminateResult :
                std::cerr << "IndeterminateResult !";
                break;
            case CalcErr::OperationNotFound :
                std::cerr << "Can't find the Obtained Operation";
                break;
            case CalcErr::NegativeRoot:
                std::cerr << "Can not take sqrt of a negative number";
            default:
                std::cerr << "Unknown Error Occurred!";
                break;
        }
    }

    return 0;
};