#include "algebraic_parser.h"
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <stack>
#include <iostream>
#include <limits>

#define MAKE_ERROR(error_code) {{}, EngineErrorResult(error_code)}
#define MAKE_SUCCESS(value)            \
    {                                  \
        EngineSuccessResult(value), {} \
    }

AlgebraicParser::AlgebraicParser()
{
    std::lock_guard<std::shared_mutex> lock(mutex_s);

    ops_["+"] = {[](const std::vector<double> &args) -> EngineResult
                 { if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch); return MAKE_SUCCESS(args[0] + args[1]); }, Precedence::AddSub};
    ops_["-"] = {[](const std::vector<double> &args) -> EngineResult
                 { if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch); return MAKE_SUCCESS(args[0] - args[1]); }, Precedence::AddSub};
    ops_["*"] = {[](const std::vector<double> &args) -> EngineResult
                 { if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch); return MAKE_SUCCESS(args[0] * args[1]); }, Precedence::MultiDiv};
    ops_["/"] = {[](const std::vector<double> &args) -> EngineResult
                 { if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch); if (args[1] == 0) return MAKE_ERROR(CalcErr::DivideByZero); return MAKE_SUCCESS(args[0] / args[1]); }, Precedence::MultiDiv};
    ops_["^"] = {[](const std::vector<double> &args) -> EngineResult
                 { if (args.size() != 2) return MAKE_ERROR(CalcErr::ArgumentMismatch); return MAKE_SUCCESS(std::pow(args[0], args[1])); }, Precedence::Pow};

    unary_ops_["sqrt"] = {[](const std::vector<double> &args) -> EngineResult { if(args[0]<0) return MAKE_ERROR(CalcErr::NegativeRoot); return MAKE_SUCCESS(std::sqrt(args[0])); }};
    unary_ops_["abs"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::abs(args[0])); }};
    unary_ops_["log"] = {[](const std::vector<double> &args) -> EngineResult { if(args[0]<=0) return MAKE_ERROR(CalcErr::DomainError); return MAKE_SUCCESS(std::log10(args[0])); }};
    unary_ops_["ln"] = {[](const std::vector<double> &args) -> EngineResult { if(args[0]<=0) return MAKE_ERROR(CalcErr::DomainError); return MAKE_SUCCESS(std::log(args[0])); }};
    unary_ops_["log2"] = {[](const std::vector<double> &args) -> EngineResult { if (args[0] <= 0) return MAKE_ERROR(CalcErr::DomainError); return MAKE_SUCCESS(std::log2(args[0])); }};
    unary_ops_["lg"] = unary_ops_["log2"];

    unary_ops_["sin"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::sin(args[0] * D2R)); }};
    unary_ops_["cos"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::cos(args[0] * D2R)); }};
    unary_ops_["tan"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::tan(args[0] * D2R)); }};
    
    unary_ops_["cot"] = {[](const std::vector<double> &args) -> EngineResult { 
        double t = std::tan(args[0] * D2R); 
        if (std::abs(t) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero); 
        return MAKE_SUCCESS(1.0 / t); 
    }};
    unary_ops_["sec"] = {[](const std::vector<double> &args) -> EngineResult { 
        double c = std::cos(args[0] * D2R); 
        if (std::abs(c) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero); 
        return MAKE_SUCCESS(1.0 / c); 
    }};
    unary_ops_["csc"] = {[](const std::vector<double> &args) -> EngineResult { 
        double s = std::sin(args[0] * D2R); 
        if (std::abs(s) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero); 
        return MAKE_SUCCESS(1.0 / s); 
    }};

    unary_ops_["asin"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (args[0] < -1.0 || args[0] > 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::asin(args[0]) * R2D); 
    }};
    unary_ops_["acos"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (args[0] < -1.0 || args[0] > 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::acos(args[0]) * R2D); 
    }};
    unary_ops_["atan"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::atan(args[0]) * R2D); }};
    
    unary_ops_["acot"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::atan(1.0 / args[0]) * R2D); }};
    unary_ops_["asec"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (std::abs(args[0]) < 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::acos(1.0 / args[0]) * R2D); 
    }};
    unary_ops_["acsc"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (std::abs(args[0]) < 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::asin(1.0 / args[0]) * R2D); 
    }};

    unary_ops_["sinh"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::sinh(args[0])); }};
    unary_ops_["cosh"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::cosh(args[0])); }};
    unary_ops_["tanh"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::tanh(args[0])); }};
    
    unary_ops_["coth"] = {[](const std::vector<double> &args) -> EngineResult { 
        double t = std::tanh(args[0]); 
        if (std::abs(t) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero); 
        return MAKE_SUCCESS(1.0 / t); 
    }};
    unary_ops_["sech"] = {[](const std::vector<double> &args) -> EngineResult { 
        double c = std::cosh(args[0]); 
        if (std::abs(c) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero); 
        return MAKE_SUCCESS(1.0 / c); 
    }};
    unary_ops_["csch"] = {[](const std::vector<double> &args) -> EngineResult { 
        double s = std::sinh(args[0]); 
        if (std::abs(s) < 1e-9) return MAKE_ERROR(CalcErr::DivideByZero); 
        return MAKE_SUCCESS(1.0 / s); 
    }};

    unary_ops_["asinh"] = {[](const std::vector<double> &args) -> EngineResult { return MAKE_SUCCESS(std::asinh(args[0])); }};
    unary_ops_["acosh"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (args[0] < 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::acosh(args[0])); 
    }};
    unary_ops_["atanh"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (std::abs(args[0]) >= 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::atanh(args[0])); 
    }};
    unary_ops_["acoth"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (std::abs(args[0]) <= 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::atanh(1.0 / args[0])); 
    }};
    unary_ops_["asech"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (args[0] <= 0.0 || args[0] > 1.0) return MAKE_ERROR(CalcErr::DomainError); 
        return MAKE_SUCCESS(std::acosh(1.0 / args[0])); 
    }};
    unary_ops_["acsch"] = {[](const std::vector<double> &args) -> EngineResult { 
        if (args[0] == 0.0) return MAKE_ERROR(CalcErr::DivideByZero); 
        return MAKE_SUCCESS(std::asinh(1.0 / args[0])); 
    }};

    RegisterSpecialCommands();
}

void AlgebraicParser::RegisterSpecialCommands() {
    special_commands_.push_back({"quadratic", [this](const std::string& s){ return HandleQuadratic(s); }});
    special_commands_.push_back({"solve_nl", [this](const std::string& s){ return HandleNonLinearSolve(s); }});
}

EngineResult AlgebraicParser::ParseAndExecute(const std::string &input) {
    std::stringstream ss(input);
    std::string first_token;
    ss >> first_token;

    for (const auto& entry : special_commands_) {
        if (first_token == entry.command) {
            return entry.handler(input);
        }
    }

    std::queue<std::string> rpn_queue = ParseToRPN(input);
    return EvaluateRPN(rpn_queue);
}

EngineResult AlgebraicParser::HandleQuadratic(const std::string& input) {
    std::stringstream ss(input);
    std::string cmd;
    double a, b, c;
    ss >> cmd >> a >> b >> c;
    if (ss.fail()) return {{}, {EngineErrorResult(CalcErr::ArgumentMismatch)}};
    return SolveQuadratic(a, b, c);
}

EngineResult AlgebraicParser::HandleNonLinearSolve(const std::string& input) {
    std::vector<std::string> eqns = {"x^2 + y^2 - 10", "x - y - 2"};
    std::map<std::string, double> guess = {{"x", 1.0}, {"y", 1.0}};
    return SolveNonLinearSystem(eqns, guess);
}

EngineResult AlgebraicParser::SolveQuadratic(double a, double b, double c) {
    if (a == 0.0) {
        if (b == 0.0) return {{}, {EngineErrorResult(CalcErr::IndeterminateResult)}};
        return {EngineSuccessResult(Vector({-c / b})), {}};
    }
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return {{}, {EngineErrorResult(CalcErr::NegativeRoot)}};
    double sqrt_d = std::sqrt(discriminant);
    double x1 = (-b + sqrt_d) / (2 * a);
    double x2 = (-b - sqrt_d) / (2 * a);
    if (std::abs(discriminant) < 1e-9) x2 = x1;
    return {EngineSuccessResult(Vector({x1, x2})), {}};
}

std::vector<double> SolveLinearSystemSmall(std::vector<std::vector<double>> A, std::vector<double> b) {
    int n = A.size();
    for (int i=0; i<n; ++i) {
        int pivot = i;
        for (int j=i+1; j<n; ++j) if (std::abs(A[j][i]) > std::abs(A[pivot][i])) pivot = j;
        std::swap(A[i], A[pivot]); std::swap(b[i], b[pivot]);
        double div = A[i][i];
        for (int j=i; j<n; ++j) A[i][j] /= div;
        b[i] /= div;
        for (int k=0; k<n; ++k) {
            if (k != i) {
                double factor = A[k][i];
                for (int j=i; j<n; ++j) A[k][j] -= factor * A[i][j];
                b[k] -= factor * b[i];
            }
        }
    }
    return b;
}

EngineResult AlgebraicParser::SolveNonLinearSystem(const std::vector<std::string>& equation_strs, std::map<std::string, double>& guess) {
    int max_iter = 50;
    double tolerance = 1e-6;
    double epsilon = 1e-5;
    
    std::vector<std::string> var_names;
    for(auto const& [key, val] : guess) var_names.push_back(key);
    int n = var_names.size(); 
    
    std::vector<std::queue<std::string>> rpns;
    for(const auto& eq : equation_strs) rpns.push_back(ParseToRPN(eq));

    for (int iter = 0; iter < max_iter; ++iter) {
        std::vector<double> F(n);
        for(int i=0; i<n; ++i) {
            auto res = EvaluateRPN(rpns[i], guess);
            if (!res.result.has_value()) return res;
            F[i] = std::get<double>(res.result.value());
        }

        double error_norm = 0;
        for(double val : F) error_norm += val*val;
        if (std::sqrt(error_norm) < tolerance) break;

        std::vector<std::vector<double>> J(n, std::vector<double>(n));
        for (int j = 0; j < n; ++j) {
            std::string var = var_names[j];
            double original_val = guess[var];
            guess[var] = original_val + epsilon;
            for (int i = 0; i < n; ++i) {
                auto res_plus = EvaluateRPN(rpns[i], guess);
                double f_plus = std::get<double>(res_plus.result.value());
                J[i][j] = (f_plus - F[i]) / epsilon;
            }
            guess[var] = original_val;
        }

        std::vector<double> neg_F = F;
        for(double& val : neg_F) val = -val;
        std::vector<double> delta = SolveLinearSystemSmall(J, neg_F);
        for(int i=0; i<n; ++i) guess[var_names[i]] += delta[i];
    }
    
    std::vector<double> final_values;
    for(const auto& name : var_names) final_values.push_back(guess[name]);
    return {EngineSuccessResult(final_values), {}};
}

void AlgebraicParser::RegisterOperator(const std::string &op, const OperatorDetails &details)
{
    std::lock_guard<std::shared_mutex> lock(mutex_s);
    ops_[op] = details;
}

void AlgebraicParser::RegisterUnaryOperator(const std::string &op, const UnaryOperatorDetails &details)
{
    std::lock_guard<std::shared_mutex> lock(mutex_s);
    unary_ops_[op] = details;
}

bool AlgebraicParser::isNumber(const std::string &token) const
{
    if (token.empty()) return false;
    char *end = nullptr;
    std::strtod(token.c_str(), &end);
    return end != token.c_str() && *end == '\0';
}

Precedence AlgebraicParser::get_precedence(const std::string &token) const
{
    if (ops_.count(token)) return ops_.at(token).precedence;
    if (unary_ops_.count(token)) return unary_ops_.at(token).precedence;
    return Precedence::None;
}

bool AlgebraicParser::isLeftAssociative(const std::string &token) const
{
    return token != "^";
}

bool AlgebraicParser::isSeparator(char c) const
{
    return c == ' ' || c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '^' || c == '%';
}

std::queue<std::string> AlgebraicParser::ParseToRPN(const std::string &expression)
{
    std::queue<std::string> output_queue;
    std::stack<std::string> operator_stack;
    std::string token;

    for (size_t i = 0; i < expression.length(); ++i)
    {
        char c = expression[i];
        if (c == ' ') continue;

        if (isdigit(c) || c == '.')
        {
            token += c;
            while (i + 1 < expression.length() && (isdigit(expression[i + 1]) || expression[i + 1] == '.'))
            {
                token += expression[++i];
            }
            output_queue.push(token);
            token.clear();
        }
        else if (isalpha(c))
        {
            token += c;
            while (i + 1 < expression.length() && isalpha(expression[i + 1]))
            {
                token += expression[++i];
            }
            if (ops_.count(token) || unary_ops_.count(token))
            {
                while (!operator_stack.empty() && operator_stack.top() != "(")
                {
                    std::string top = operator_stack.top();
                    if ((isLeftAssociative(token) && get_precedence(token) <= get_precedence(top)) ||
                        (!isLeftAssociative(token) && get_precedence(token) < get_precedence(top)))
                    {
                        output_queue.push(top);
                        operator_stack.pop();
                    }
                    else break;
                }
                operator_stack.push(token);
            } else {
                output_queue.push(token); 
            }
            token.clear();
        }
        else if (isSeparator(c))
        {
            std::string op(1, c);
            if (c == '(')
            {
                operator_stack.push(op);
            }
            else if (c == ')')
            {
                while (!operator_stack.empty() && operator_stack.top() != "(")
                {
                    output_queue.push(operator_stack.top());
                    operator_stack.pop();
                }
                if (!operator_stack.empty()) operator_stack.pop();
            }
            else
            {
                while (!operator_stack.empty() && operator_stack.top() != "(")
                {
                    std::string top = operator_stack.top();
                    if ((isLeftAssociative(op) && get_precedence(op) <= get_precedence(top)) ||
                        (!isLeftAssociative(op) && get_precedence(op) < get_precedence(top)))
                    {
                        output_queue.push(top);
                        operator_stack.pop();
                    }
                    else break;
                }
                operator_stack.push(op);
            }
        }
    }

    while (!operator_stack.empty())
    {
        output_queue.push(operator_stack.top());
        operator_stack.pop();
    }

    return output_queue;
}

EngineResult AlgebraicParser::EvaluateRPN(std::queue<std::string> &rpn_queue, const std::map<std::string, double>& variables)
{
    std::stack<double> value_stack;

    while (!rpn_queue.empty())
    {
        std::string token = rpn_queue.front();
        rpn_queue.pop();

        if (isNumber(token)) {
            value_stack.push(std::stod(token));
        }
        else if (std::isalpha(token[0])) { 
            if (variables.find(token) != variables.end()) {
                value_stack.push(variables.at(token));
            }
            else if (unary_ops_.count(token)) {
                 if (value_stack.empty()) return MAKE_ERROR(CalcErr::ArgumentMismatch);
                 double val = value_stack.top(); value_stack.pop();
                 EngineResult op_res = unary_ops_.at(token).operation({val});
                 if (op_res.error.has_value()) return op_res;
                 value_stack.push(std::get<double>(op_res.result.value()));
            }
            else {
                return MAKE_ERROR(CalcErr::ArgumentMismatch);
            }
        }
        else if (ops_.count(token))
        {
            if (value_stack.size() < 2) return MAKE_ERROR(CalcErr::ArgumentMismatch);
            double val2 = value_stack.top(); value_stack.pop();
            double val1 = value_stack.top(); value_stack.pop();
            EngineResult op_res = ops_.at(token).operation({val1, val2});
            if (op_res.error.has_value()) return op_res;
            value_stack.push(std::get<double>(op_res.result.value()));
        }
    }

    if (value_stack.size() != 1) return MAKE_ERROR(CalcErr::ArgumentMismatch);
    return MAKE_SUCCESS(value_stack.top());
}