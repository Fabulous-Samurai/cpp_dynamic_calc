/**
 * @file algebraic_parser.cpp
 * @brief Implementation of the Algebraic Parser using AST.
 * RESTORED: Full functionality including Evaluate, Derivative, and Simplify logic.
 */

#include "algebraic_parser.h"
#include "string_helpers.h"
#include <exception>
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>

// --- Helpers ---

Precedence GetOpPrecedence(char op) {
    if (op == '+' || op == '-') return Precedence::AddSub;
    if (op == '*' || op == '/') return Precedence::MultiDiv;
    if (op == '^') return Precedence::Pow;
    return Precedence::None;
}


bool IsConst(const NodePtr node, double val) {
    auto res = node->Evaluate({});
    if (!res.value.has_value()) return false;
    return std::abs(*res.value - val) < 1e-9;
}

CalcErr NormalizeError(const EvalResult& res, CalcErr fallback = CalcErr::ArgumentMismatch) {
    return res.error == CalcErr::None ? fallback : res.error;
}

std::string FormatNumber(double val) {
    // Handle special cases
    if (std::isinf(val)) return std::signbit(val) ? "-inf" : "inf";
    if (std::isnan(val)) return "nan";
    
    // For integers or numbers that can be represented exactly
    if (val == std::floor(val) && std::abs(val) < 1e15) {
        return std::to_string(static_cast<long long>(val));
    }
    
    // For normal floating point numbers
    char buffer[64];
    double abs_val = std::abs(val);
    
    if (abs_val >= 1e6 || (abs_val > 0 && abs_val < 1e-6)) {
        // Use scientific notation for very large or very small numbers
        sprintf(buffer, "%.6e", val);
    } else {
        // Use high precision for normal numbers - let's try 15 digits
        sprintf(buffer, "%.15g", val);
    }
    
    return std::string(buffer);
}

// ========================================================
// AST NODE IMPLEMENTATIONS
// ========================================================

struct NumberNode : ExprNode {
    double value;
    NumberNode(double v) : value(v) {}
    
    
    EvalResult Evaluate(const std::map<std::string, double>&) const override { return EvalResult::Success(value); }
    
    NodePtr Derivative(Arena& arena, std::string_view) const override { return arena.alloc<NumberNode>(0.0); }
    NodePtr Simplify(Arena& arena) const override { return arena.alloc<NumberNode>(value); }
    std::string ToString(Precedence) const override { return FormatNumber(value); }
};

struct VariableNode : ExprNode {
    std::string_view name;
    VariableNode(std::string_view n) : name(n) {}
    
    EvalResult Evaluate(const std::map<std::string, double>& vars) const override {
        std::string key(name);
        auto it = vars.find(key);
        if (it != vars.end()) return EvalResult::Success(it->second);
        if (key == "Ans") return EvalResult::Success(0.0);
        
        // Mathematical constants
        if (key == "pi" || key == "PI") return EvalResult::Success(PI_CONST);
        if (key == "e" || key == "E") return EvalResult::Success(2.718281828459045);
        if (key == "phi") return EvalResult::Success(1.618033988749895); // Golden ratio
        
        return EvalResult::Failure(CalcErr::ArgumentMismatch);
    }
    
    NodePtr Derivative(Arena& arena, std::string_view var) const override {
        if (name == var) return arena.alloc<NumberNode>(1.0);
        return arena.alloc<NumberNode>(0.0);
    }
    NodePtr Simplify(Arena& arena) const override { return arena.alloc<VariableNode>(name); }
    std::string ToString(Precedence) const override { return std::string(name); }
};

struct BinaryOpNode : ExprNode {
    char op; NodePtr left, right;
    BinaryOpNode(char c, NodePtr l, NodePtr r) : op(c), left(l), right(r) {}
    
    // [KRİTİK] Bu fonksiyonu silersen NaN alırsın!
    EvalResult Evaluate(const std::map<std::string, double>& vars) const override {
        auto left_eval = left->Evaluate(vars);
        if (!left_eval.HasValue()) return left_eval;
        auto right_eval = right->Evaluate(vars);
        if (!right_eval.HasValue()) return right_eval;
        double l = *left_eval.value;
        double r = *right_eval.value;
        switch(op) {
            case '+': {
                auto safe_result = SafeMath::SafeAdd(l, r);
                return safe_result ? EvalResult::Success(*safe_result) : EvalResult::Failure(CalcErr::NumericOverflow);
            }
            case '-': {
                auto safe_result = SafeMath::SafeAdd(l, -r);
                return safe_result ? EvalResult::Success(*safe_result) : EvalResult::Failure(CalcErr::NumericOverflow);
            }
            case '*': {
                if (!SafeMath::IsFiniteAndSafe(l * r)) return EvalResult::Failure(CalcErr::NumericOverflow);
                return EvalResult::Success(l * r);
            }
            case '/': {
                if (r == 0.0) return EvalResult::Failure(CalcErr::DivideByZero);
                if (!SafeMath::IsFiniteAndSafe(l / r)) return EvalResult::Failure(CalcErr::NumericOverflow);
                return EvalResult::Success(l / r);
            }
            case '^': {
                auto safe_result = SafeMath::SafePow(l, r);
                return safe_result ? EvalResult::Success(*safe_result) : EvalResult::Failure(CalcErr::NumericOverflow);
            }
            default: return EvalResult::Success(0.0);
        }
    }
    
    NodePtr Derivative(Arena& arena, std::string_view var) const override {
        auto dl = left->Derivative(arena, var);
        auto dr = right->Derivative(arena, var);
        
        if (op == '+' || op == '-') return arena.alloc<BinaryOpNode>(op, dl, dr);
        if (op == '*') {
            auto t1 = arena.alloc<BinaryOpNode>('*', dl, right);
            auto t2 = arena.alloc<BinaryOpNode>('*', left, dr);
            return arena.alloc<BinaryOpNode>('+', t1, t2);
        }
        if (op == '/') {
            auto t1 = arena.alloc<BinaryOpNode>('*', dl, right);
            auto t2 = arena.alloc<BinaryOpNode>('*', left, dr);
            auto num = arena.alloc<BinaryOpNode>('-', t1, t2);
            auto den = arena.alloc<BinaryOpNode>('^', right, arena.alloc<NumberNode>(2.0));
            return arena.alloc<BinaryOpNode>('/', num, den);
        }
        if (op == '^') {
            auto n_minus_1 = arena.alloc<BinaryOpNode>('-', right, arena.alloc<NumberNode>(1.0));
            auto u_pow = arena.alloc<BinaryOpNode>('^', left, n_minus_1);
            auto n_times_u = arena.alloc<BinaryOpNode>('*', right, u_pow);
            return arena.alloc<BinaryOpNode>('*', n_times_u, dl);
        }
        return arena.alloc<NumberNode>(0.0); 
    }

    NodePtr Simplify(Arena& arena) const override {
        auto simple_left = left->Simplify(arena);
        auto simple_right = right->Simplify(arena);

       
        bool l_const = false, r_const = false;
        double l_val = 0, r_val = 0;
        auto l_eval = simple_left->Evaluate({});
        if (l_eval.value.has_value()) { l_const = true; l_val = *l_eval.value; }
        auto r_eval = simple_right->Evaluate({});
        if (r_eval.value.has_value()) { r_const = true; r_val = *r_eval.value; }

        if (l_const && r_const) {
            if (op == '+') return arena.alloc<NumberNode>(l_val + r_val);
            if (op == '-') return arena.alloc<NumberNode>(l_val - r_val);
            if (op == '*') return arena.alloc<NumberNode>(l_val * r_val);
            if (op == '/' && r_val != 0) return arena.alloc<NumberNode>(l_val / r_val);
            if (op == '^') return arena.alloc<NumberNode>(std::pow(l_val, r_val));
        }

        if (op == '+') {
            if (IsConst(simple_right, 0.0)) return simple_left;
            if (IsConst(simple_left, 0.0)) return simple_right;
            if (simple_left->ToString(Precedence::None) == simple_right->ToString(Precedence::None)) 
                return arena.alloc<BinaryOpNode>('*', arena.alloc<NumberNode>(2.0), simple_left);
        }
        else if (op == '*') {
            if (IsConst(simple_right, 0.0) || IsConst(simple_left, 0.0)) return arena.alloc<NumberNode>(0.0);
            if (IsConst(simple_right, 1.0)) return simple_left;
            if (IsConst(simple_left, 1.0)) return simple_right;
        }
        else if (op == '^') {
            if (IsConst(simple_right, 1.0)) return simple_left;
            if (IsConst(simple_right, 0.0)) return arena.alloc<NumberNode>(1.0);
        }
        else if (op == '/') {
            if (IsConst(simple_left, 0.0)) return arena.alloc<NumberNode>(0.0);
            if (IsConst(simple_right, 1.0)) return simple_left;
            if (simple_left->ToString(Precedence::None) == simple_right->ToString(Precedence::None)) 
                return arena.alloc<NumberNode>(1.0);
        }

        return arena.alloc<BinaryOpNode>(op, simple_left, simple_right);
    }

    std::string ToString(Precedence parent_prec) const override {
        Precedence my_prec = GetOpPrecedence(op);
        std::string result = left->ToString(my_prec) + " " + op + " " + right->ToString(my_prec); 
        if (static_cast<int>(my_prec) < static_cast<int>(parent_prec)) return "(" + result + ")";
        return result;
    }
};

struct UnaryOpNode : ExprNode {
    std::string_view func; NodePtr operand;
    UnaryOpNode(std::string_view f, NodePtr op) : func(f), operand(op) {}
    
  
    EvalResult Evaluate(const std::map<std::string, double>& vars) const override {
        auto inner = operand->Evaluate(vars);
        if (!inner.HasValue()) return inner;
        double val = *inner.value;
        if (func == "sin") return EvalResult::Success(std::sin(val * D2R));
        if (func == "cos") return EvalResult::Success(std::cos(val * D2R));
        if (func == "tan") return EvalResult::Success(std::tan(val * D2R));
        if (func == "cot") return EvalResult::Success(1.0 / std::tan(val * D2R));
        if (func == "sec") return EvalResult::Success(1.0 / std::cos(val * D2R));
        if (func == "csc") return EvalResult::Success(1.0 / std::sin(val * D2R));
        
        if (func == "asin") return EvalResult::Success(std::asin(val) * R2D);
        if (func == "acos") return EvalResult::Success(std::acos(val) * R2D);
        if (func == "atan") return EvalResult::Success(std::atan(val) * R2D);
        if (func == "acot") return EvalResult::Success(std::atan(1.0 / val) * R2D);
        if (func == "asec") return EvalResult::Success(std::acos(1.0 / val) * R2D);
        if (func == "acsc") return EvalResult::Success(std::asin(1.0 / val) * R2D);
        
        if (func == "sinh") return EvalResult::Success(std::sinh(val));
        if (func == "cosh") return EvalResult::Success(std::cosh(val));
        if (func == "tanh") return EvalResult::Success(std::tanh(val));
        if (func == "coth") return EvalResult::Success(1.0 / std::tanh(val));
        if (func == "sech") return EvalResult::Success(1.0 / std::cosh(val));
        if (func == "csch") return EvalResult::Success(1.0 / std::sinh(val));
        
        if (func == "asinh") return EvalResult::Success(std::asinh(val));
        if (func == "acosh") return EvalResult::Success(std::acosh(val));
        if (func == "atanh") return EvalResult::Success(std::atanh(val));
        if (func == "acoth") return EvalResult::Success(std::atanh(1.0 / val));
        if (func == "asech") return EvalResult::Success(std::asinh(1.0 / val));
        if (func == "acsch") return EvalResult::Success(std::asinh(1.0 / val));

        if (func == "sqrt") {
            if (val < 0) return EvalResult::Failure(CalcErr::NegativeRoot);
            return EvalResult::Success(std::sqrt(val));
        }
        if (func == "cbrt") return EvalResult::Success(std::cbrt(val));
        if (func == "abs") return EvalResult::Success(std::abs(val));
        if (func == "ln") {
            if (val <= 0) return EvalResult::Failure(CalcErr::DomainError);
            return EvalResult::Success(std::log(val));
        }
        if (func == "log") {
            if (val <= 0) return EvalResult::Failure(CalcErr::DomainError);
            return EvalResult::Success(std::log10(val));
        }
        if (func == "log2" || func == "lg") {
            if (val <= 0) return EvalResult::Failure(CalcErr::DomainError);
            return EvalResult::Success(std::log2(val));
        }
        if (func == "exp") return EvalResult::Success(std::exp(val));
        
        // Additional mathematical functions
        if (func == "factorial") {
            if (val < 0 || val != std::floor(val) || val > 170) return EvalResult::Failure(CalcErr::DomainError);
            double result = 1.0;
            for (int i = 2; i <= static_cast<int>(val); ++i) {
                result *= i;
            }
            return EvalResult::Success(result);
        }
        
        if (func == "u-") return EvalResult::Success(-val);
        return EvalResult::Success(0.0);
    }
    
    NodePtr Derivative(Arena& arena, std::string_view var) const override {
        auto d_inner = operand->Derivative(arena, var);
        if (func == "u-") return arena.alloc<UnaryOpNode>("u-", d_inner);
        
        if (func == "sin") {
            auto cos_u = arena.alloc<UnaryOpNode>("cos", operand);
            return arena.alloc<BinaryOpNode>('*', cos_u, d_inner);
        }
        if (func == "cos") {
            auto sin_u = arena.alloc<UnaryOpNode>("sin", operand);
            auto neg_sin = arena.alloc<UnaryOpNode>("u-", sin_u);
            return arena.alloc<BinaryOpNode>('*', neg_sin, d_inner);
        }
        if (func == "tan") {
            auto sec_u = arena.alloc<UnaryOpNode>("sec", operand);
            auto sec_sq = arena.alloc<BinaryOpNode>('^', sec_u, arena.alloc<NumberNode>(2.0));
            return arena.alloc<BinaryOpNode>('*', sec_sq, d_inner);
        }
        if (func == "cot") {
            auto csc_u = arena.alloc<UnaryOpNode>("csc", operand);
            auto csc_sq = arena.alloc<BinaryOpNode>('^', csc_u, arena.alloc<NumberNode>(2.0));
            auto neg = arena.alloc<UnaryOpNode>("u-", csc_sq);
            return arena.alloc<BinaryOpNode>('*', neg, d_inner);
        }
        if (func == "sec") {
            auto sec_u = arena.alloc<UnaryOpNode>("sec", operand);
            auto tan_u = arena.alloc<UnaryOpNode>("tan", operand);
            auto prod = arena.alloc<BinaryOpNode>('*', sec_u, tan_u);
            return arena.alloc<BinaryOpNode>('*', prod, d_inner);
        }
        if (func == "csc") {
            auto csc_u = arena.alloc<UnaryOpNode>("csc", operand);
            auto cot_u = arena.alloc<UnaryOpNode>("cot", operand);
            auto prod = arena.alloc<BinaryOpNode>('*', csc_u, cot_u);
            auto neg = arena.alloc<UnaryOpNode>("u-", prod);
            return arena.alloc<BinaryOpNode>('*', neg, d_inner);
        }
        if (func == "ln") return arena.alloc<BinaryOpNode>('/', d_inner, operand);
        if (func == "log2" || func == "lg") {
            auto ln2 = arena.alloc<NumberNode>(std::log(2.0));
            auto denom = arena.alloc<BinaryOpNode>('*', operand, ln2);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "sqrt") {
            auto two = arena.alloc<NumberNode>(2.0);
            auto sqrt_u = arena.alloc<UnaryOpNode>("sqrt", operand);
            auto denom = arena.alloc<BinaryOpNode>('*', two, sqrt_u);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "exp") {
            auto exp_u = arena.alloc<UnaryOpNode>("exp", operand);
            return arena.alloc<BinaryOpNode>('*', exp_u, d_inner);
        }
        if (func == "asin") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto radicand = arena.alloc<BinaryOpNode>('-', one, inner_sq);
            auto denom = arena.alloc<UnaryOpNode>("sqrt", radicand);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "acos") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto radicand = arena.alloc<BinaryOpNode>('-', one, inner_sq);
            auto denom = arena.alloc<UnaryOpNode>("sqrt", radicand);
            auto neg = arena.alloc<UnaryOpNode>("u-", arena.alloc<BinaryOpNode>('/', d_inner, denom));
            return neg;
        }
        if (func == "atan") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto denom = arena.alloc<BinaryOpNode>('+', one, inner_sq);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "acot") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto denom = arena.alloc<BinaryOpNode>('-', inner_sq, one);
            auto neg = arena.alloc<UnaryOpNode>("u-", arena.alloc<BinaryOpNode>('/', d_inner, denom));
            return neg;
        }
        if (func == "asec") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto radicand = arena.alloc<BinaryOpNode>('-', inner_sq, one);
            auto sqrt = arena.alloc<UnaryOpNode>("sqrt", radicand);
            auto denom = arena.alloc<BinaryOpNode>('*', operand, sqrt);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "acsc") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto radicand = arena.alloc<BinaryOpNode>('-', inner_sq, one);
            auto sqrt = arena.alloc<UnaryOpNode>("sqrt", radicand);
            auto denom = arena.alloc<BinaryOpNode>('*', operand, sqrt);
            auto neg = arena.alloc<UnaryOpNode>("u-", arena.alloc<BinaryOpNode>('/', d_inner, denom));
            return neg;
        }
        if (func == "sinh") {
            auto cosh_u = arena.alloc<UnaryOpNode>("cosh", operand);
            return arena.alloc<BinaryOpNode>('*', cosh_u, d_inner);
        }
        if (func == "cosh") {
            auto sinh_u = arena.alloc<UnaryOpNode>("sinh", operand);
            return arena.alloc<BinaryOpNode>('*', sinh_u, d_inner);
        }
        if (func == "tanh") {
            auto sech_u = arena.alloc<UnaryOpNode>("sech", operand);
            auto sech_sq = arena.alloc<BinaryOpNode>('^', sech_u, arena.alloc<NumberNode>(2.0));
            return arena.alloc<BinaryOpNode>('*', sech_sq, d_inner);
        }
        if (func == "coth") {
            auto csch_u = arena.alloc<UnaryOpNode>("csch", operand);
            auto csch_sq = arena.alloc<BinaryOpNode>('^', csch_u, arena.alloc<NumberNode>(2.0));
            auto neg = arena.alloc<UnaryOpNode>("u-", csch_sq);
            return arena.alloc<BinaryOpNode>('*', neg, d_inner);
        }
        if (func == "sech") {
            auto sech_u = arena.alloc<UnaryOpNode>("sech", operand);
            auto tanh_u = arena.alloc<UnaryOpNode>("tanh", operand);
            auto prod = arena.alloc<BinaryOpNode>('*', sech_u, tanh_u);
            auto neg = arena.alloc<UnaryOpNode>("u-", prod);
            return arena.alloc<BinaryOpNode>('*', neg, d_inner);
        }
        if (func == "csch") {
            auto csch_u = arena.alloc<UnaryOpNode>("csch", operand);
            auto coth_u = arena.alloc<UnaryOpNode>("coth", operand);
            auto prod = arena.alloc<BinaryOpNode>('*', csch_u, coth_u);
            auto neg = arena.alloc<UnaryOpNode>("u-", prod);
            return arena.alloc<BinaryOpNode>('*', neg, d_inner);
        }
        if (func == "asinh") {
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto radicand = arena.alloc<BinaryOpNode>('+', inner_sq, arena.alloc<NumberNode>(1.0));
            auto sqrt = arena.alloc<UnaryOpNode>("sqrt", radicand);
            return arena.alloc<BinaryOpNode>('/', d_inner, sqrt);
        }
        if (func == "acosh") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto minus = arena.alloc<BinaryOpNode>('-', operand, one);
            auto plus = arena.alloc<BinaryOpNode>('+', operand, one);
            auto sqrt1 = arena.alloc<UnaryOpNode>("sqrt", minus);
            auto sqrt2 = arena.alloc<UnaryOpNode>("sqrt", plus);
            auto denom = arena.alloc<BinaryOpNode>('*', sqrt1, sqrt2);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "atanh") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto denom = arena.alloc<BinaryOpNode>('-', one, inner_sq);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "acoth") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto denom = arena.alloc<BinaryOpNode>('-', one, inner_sq);
            return arena.alloc<BinaryOpNode>('/', d_inner, denom);
        }
        if (func == "asech") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto radicand = arena.alloc<BinaryOpNode>('-', one, inner_sq);
            auto sqrt = arena.alloc<UnaryOpNode>("sqrt", radicand);
            auto denom = arena.alloc<BinaryOpNode>('*', operand, sqrt);
            auto neg = arena.alloc<UnaryOpNode>("u-", arena.alloc<BinaryOpNode>('/', d_inner, denom));
            return neg;
        }
        if (func == "acsch") {
            auto one = arena.alloc<NumberNode>(1.0);
            auto inner_sq = arena.alloc<BinaryOpNode>('^', operand, arena.alloc<NumberNode>(2.0));
            auto radicand = arena.alloc<BinaryOpNode>('+', inner_sq, one);
            auto sqrt = arena.alloc<UnaryOpNode>("sqrt", radicand);
            auto denom = arena.alloc<BinaryOpNode>('*', operand, sqrt);
            auto neg = arena.alloc<UnaryOpNode>("u-", arena.alloc<BinaryOpNode>('/', d_inner, denom));
            return neg;
        }
        
        return arena.alloc<NumberNode>(0.0);
    }

    NodePtr Simplify(Arena& arena) const override {
        auto simple_inner = operand->Simplify(arena);
        return arena.alloc<UnaryOpNode>(func, simple_inner);
    }

    std::string ToString(Precedence) const override {
        if (func == "u-") return "-" + operand->ToString(Precedence::Unary);
        return std::string(func) + "(" + operand->ToString(Precedence::None) + ")";
    }
};

// ========================================================
// MULTI-ARGUMENT FUNCTION NODE (FOR CALCULUS OPERATIONS)
// ========================================================
struct MultiArgFunctionNode : ExprNode {
    std::string_view func;
    std::vector<NodePtr> args;
    
    MultiArgFunctionNode(std::string_view f, std::vector<NodePtr> arguments) 
        : func(f), args(std::move(arguments)) {}
    
    EvalResult Evaluate(const std::map<std::string, double>& vars) const override {
        if (func == "limit") {
            if (args.size() != 3) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            
            // limit(expression, variable, point)
            // Use epsilon-delta numerical limit calculation
            auto var_node = dynamic_cast<VariableNode*>(args[1]);
            if (!var_node) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            
            std::string var_name = std::string(var_node->name);
            auto point_result = args[2]->Evaluate(vars);
            if (!point_result.HasValue()) return point_result;
            double approach_point = *point_result.value;
            
            // Check for infinite limit
            if (std::isinf(approach_point)) {
                return EvaluateLimitAtInfinity(vars, var_name, approach_point > 0);
            }
            
            return EvaluateNumericalLimit(vars, var_name, approach_point);
        }
        
        if (func == "integrate") {
            if (args.size() != 4) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            
            // integrate(expression, variable, lower_bound, upper_bound)
            auto var_node = dynamic_cast<VariableNode*>(args[1]);
            if (!var_node) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            
            std::string var_name = std::string(var_node->name);
            auto lower_result = args[2]->Evaluate(vars);
            auto upper_result = args[3]->Evaluate(vars);
            
            if (!lower_result.HasValue() || !upper_result.HasValue()) {
                return EvalResult::Failure(CalcErr::DomainError);
            }
            
            double a = *lower_result.value;
            double b = *upper_result.value;
            
            // Check for improper integrals
            if (std::isinf(a) || std::isinf(b)) {
                return EvaluateImproperIntegral(vars, var_name, a, b);
            }
            
            return EvaluateNumericalIntegral(vars, var_name, a, b);
        }
        
        // Basic multi-argument functions
        if (func == "max") {
            if (args.empty()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            double max_val = -std::numeric_limits<double>::infinity();
            for (const auto& arg : args) {
                auto result = arg->Evaluate(vars);
                if (!result.HasValue()) return result;
                max_val = std::max(max_val, *result.value);
            }
            return EvalResult::Success(max_val);
        }
        
        if (func == "min") {
            if (args.empty()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            double min_val = std::numeric_limits<double>::infinity();
            for (const auto& arg : args) {
                auto result = arg->Evaluate(vars);
                if (!result.HasValue()) return result;
                min_val = std::min(min_val, *result.value);
            }
            return EvalResult::Success(min_val);
        }
        
        if (func == "gcd") {
            if (args.size() != 2) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            auto a_result = args[0]->Evaluate(vars);
            auto b_result = args[1]->Evaluate(vars);
            if (!a_result.HasValue() || !b_result.HasValue()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            
            long long a = static_cast<long long>(*a_result.value);
            long long b = static_cast<long long>(*b_result.value);
            a = std::abs(a); b = std::abs(b);
            
            while (b != 0) {
                long long temp = b;
                b = a % b;
                a = temp;
            }
            return EvalResult::Success(static_cast<double>(a));
        }
        
        if (func == "lcm") {
            if (args.size() != 2) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            auto a_result = args[0]->Evaluate(vars);
            auto b_result = args[1]->Evaluate(vars);
            if (!a_result.HasValue() || !b_result.HasValue()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            
            long long a = static_cast<long long>(*a_result.value);
            long long b = static_cast<long long>(*b_result.value);
            a = std::abs(a); b = std::abs(b);
            
            if (a == 0 || b == 0) return EvalResult::Success(0.0);
            
            // Calculate GCD first
            long long gcd_val = a;
            long long temp_b = b;
            while (temp_b != 0) {
                long long temp = temp_b;
                temp_b = gcd_val % temp_b;
                gcd_val = temp;
            }
            
            long long lcm_val = (a / gcd_val) * b;
            return EvalResult::Success(static_cast<double>(lcm_val));
        }
        
        if (func == "mod" || func == "modulo") {
            if (args.size() != 2) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            auto a_result = args[0]->Evaluate(vars);
            auto b_result = args[1]->Evaluate(vars);
            if (!a_result.HasValue() || !b_result.HasValue()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            
            double a = *a_result.value;
            double b = *b_result.value;
            if (b == 0) return EvalResult::Failure(CalcErr::DivideByZero);
            
            return EvalResult::Success(std::fmod(a, b));
        }
        
        return EvalResult::Failure(CalcErr::OperationNotFound);
    }
    
    NodePtr Derivative(Arena& arena, std::string_view var) const override {
        // Fundamental Theorem of Calculus for integrals
        if (func == "integrate" && args.size() == 4) {
            auto var_node = dynamic_cast<VariableNode*>(args[1]);
            if (var_node && var_node->name == var) {
                // d/dx ∫[a(x)]^[b(x)] f(t) dt = f(b(x))·b'(x) - f(a(x))·a'(x)
                auto f_at_b = args[0]; // Need to substitute var with upper bound
                auto f_at_a = args[0]; // Need to substitute var with lower bound
                auto b_prime = args[3]->Derivative(arena, var);
                auto a_prime = args[2]->Derivative(arena, var);
                
                // This is a simplified implementation
                // In practice, we'd need proper substitution
                return arena.alloc<NumberNode>(0.0); // Placeholder
            }
        }
        
        // For limits, derivative is complex and context-dependent
        return arena.alloc<NumberNode>(0.0); // Placeholder
    }
    
    NodePtr Simplify(Arena& arena) const override {
        std::vector<NodePtr> simplified_args;
        for (const auto& arg : args) {
            simplified_args.push_back(arg->Simplify(arena));
        }
        return arena.alloc<MultiArgFunctionNode>(func, std::move(simplified_args));
    }
    
    std::string ToString(Precedence) const override {
        std::string result = std::string(func) + "(";
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) result += ", ";
            result += args[i]->ToString(Precedence::None);
        }
        result += ")";
        return result;
    }

private:
    EvalResult EvaluateNumericalLimit(const std::map<std::string, double>& vars, 
                                    const std::string& var_name, double approach_point) const {
        constexpr double epsilon = 1e-6;  // Relaxed tolerance
        constexpr int max_iterations = 20; // Reduced iterations for faster convergence
        
        auto evaluate_at = [&](double x) -> std::optional<double> {
            std::map<std::string, double> local_vars = vars;
            local_vars[var_name] = x;
            auto result = args[0]->Evaluate(local_vars);
            return result.HasValue() ? std::optional<double>(*result.value) : std::nullopt;
        };
        
        // Try direct evaluation first (for continuous functions)
        auto direct_eval = evaluate_at(approach_point);
        if (direct_eval.has_value() && std::isfinite(*direct_eval)) {
            return EvalResult::Success(*direct_eval);
        }
        
        // Approach from both sides with progressively smaller steps
        std::optional<double> left_limit, right_limit;
        
        for (int i = 1; i <= max_iterations; ++i) {
            double h = std::pow(0.1, i);  // More aggressive step reduction
            
            // Left approach
            auto left_val = evaluate_at(approach_point - h);
            if (left_val.has_value() && std::isfinite(*left_val)) {
                left_limit = *left_val;
            }
            
            // Right approach  
            auto right_val = evaluate_at(approach_point + h);
            if (right_val.has_value() && std::isfinite(*right_val)) {
                right_limit = *right_val;
            }
            
            // Check convergence
            if (left_limit.has_value() && right_limit.has_value()) {
                if (std::abs(*left_limit - *right_limit) < epsilon) {
                    return EvalResult::Success((*left_limit + *right_limit) / 2.0);
                }
            }
        }
        
        // Return one-sided limit if available
        if (left_limit.has_value()) return EvalResult::Success(*left_limit);
        if (right_limit.has_value()) return EvalResult::Success(*right_limit);
        
        return EvalResult::Failure(CalcErr::IndeterminateResult);
    }
    
    EvalResult EvaluateLimitAtInfinity(const std::map<std::string, double>& vars,
                                     const std::string& var_name, bool positive_infinity) const {
        constexpr int max_iterations = 20;
        
        auto evaluate_at = [&](double x) -> std::optional<double> {
            std::map<std::string, double> local_vars = vars;
            local_vars[var_name] = x;
            auto result = args[0]->Evaluate(local_vars);
            return result.HasValue() ? std::optional<double>(*result.value) : std::nullopt;
        };
        
        std::optional<double> prev_val;
        
        for (int i = 1; i <= max_iterations; ++i) {
            double x = positive_infinity ? std::pow(10.0, i) : -std::pow(10.0, i);
            auto current_val = evaluate_at(x);
            
            if (!current_val.has_value() || !std::isfinite(*current_val)) {
                continue;
            }
            
            if (prev_val.has_value()) {
                double diff = std::abs(*current_val - *prev_val);
                if (diff < 1e-10) {
                    return EvalResult::Success(*current_val);
                }
            }
            prev_val = *current_val;
        }
        
        // Check for infinite limits
        if (prev_val.has_value()) {
            if (std::abs(*prev_val) > 1e10) {
                return EvalResult::Success(positive_infinity ? 
                    std::numeric_limits<double>::infinity() : 
                    -std::numeric_limits<double>::infinity());
            }
            return EvalResult::Success(*prev_val);
        }
        
        return EvalResult::Failure(CalcErr::IndeterminateResult);
    }
    
    EvalResult EvaluateNumericalIntegral(const std::map<std::string, double>& vars,
                                       const std::string& var_name, double a, double b) const {
        // Adaptive Simpson's Rule with error control
        constexpr double tolerance = 1e-12;
        constexpr int max_recursion = 15;
        
        auto f = [&](double x) -> double {
            std::map<std::string, double> local_vars = vars;
            local_vars[var_name] = x;
            auto result = args[0]->Evaluate(local_vars);
            return result.HasValue() ? *result.value : 0.0;
        };
        
        std::function<double(double, double, double, double, double, int)> simpson_adaptive = 
            [&](double a, double b, double fa, double fb, double fc, int depth) -> double {
                
            double h = (b - a) / 2.0;
            double c = a + h;
            double fd = f(a + h/2.0);
            double fe = f(c + h/2.0);
            
            double S1 = h/3.0 * (fa + 4*fc + fb);  // Original estimate
            double S2 = h/6.0 * (fa + 4*fd + 2*fc + 4*fe + fb);  // Refined estimate
            
            if (depth >= max_recursion || std::abs(S2 - S1) < 15*tolerance) {
                return S2 + (S2 - S1)/15.0;  // Richardson extrapolation
            }
            
            return simpson_adaptive(a, c, fa, fc, fd, depth+1) + 
                   simpson_adaptive(c, b, fc, fb, fe, depth+1);
        };
        
        try {
            double fa = f(a);
            double fb = f(b);
            double fc = f((a + b) / 2.0);
            
            // Check for discontinuities or infinite values
            if (!std::isfinite(fa) || !std::isfinite(fb) || !std::isfinite(fc)) {
                return EvalResult::Failure(CalcErr::DomainError);
            }
            
            double result = simpson_adaptive(a, b, fa, fb, fc, 0);
            return EvalResult::Success(result);
            
        } catch (...) {
            return EvalResult::Failure(CalcErr::DomainError);
        }
    }
    
    EvalResult EvaluateImproperIntegral(const std::map<std::string, double>& vars,
                                      const std::string& var_name, double a, double b) const {
        // Handle improper integrals by taking limits
        constexpr double large_val = 1e6;
        
        double effective_a = std::isinf(a) ? (a > 0 ? large_val : -large_val) : a;
        double effective_b = std::isinf(b) ? (b > 0 ? large_val : -large_val) : b;
        
        return EvaluateNumericalIntegral(vars, var_name, effective_a, effective_b);
    }
};

// ========================================================
// ALGEBRAIC PARSER IMPLEMENTATION
// ========================================================

AlgebraicParser::AlgebraicParser() { RegisterSpecialCommands(); }

void AlgebraicParser::RegisterSpecialCommands() {
    special_commands_.push_back({"quadratic", [this](const std::string& s){ return HandleQuadratic(s); }});
    special_commands_.push_back({"solve_nl", [this](const std::string& s){ return HandleNonLinearSolve(s); }});
    special_commands_.push_back({"derive", [this](const std::string& s){ return HandleDerivative(s); }});
}

NodePtr AlgebraicParser::ParseExpression(std::string_view input) {
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.front()))) input.remove_prefix(1);
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.back()))) input.remove_suffix(1);

    auto parse_binary = [&](std::string_view operators, bool right_to_left) -> NodePtr {
        int bracket_depth = 0;
        int start = right_to_left ? input.size() - 1 : 0;
        int end = right_to_left ? -1 : input.size();
        int step = right_to_left ? -1 : 1;

        for (int i = start; i != end; i += step) {
            char c = input[i];
            if (c == ')') bracket_depth++;
            else if (c == '(') bracket_depth--;
            else if (bracket_depth == 0) {
                if (operators.find(c) != std::string_view::npos) {
                    // Check if this is actually a unary operator (specifically for +/-)
                    if ((c == '-' || c == '+') && i == 0) {
                        // This is a unary operator at the start of expression
                        continue; // Skip this one for binary parsing
                    }
                    if ((c == '-' || c == '+') && i > 0) {
                        // Check if previous character suggests this might be unary
                        char prev = input[i-1];
                        if (prev == '(' || prev == '+' || prev == '-' || prev == '*' || prev == '/' || prev == '^') {
                            // This looks like a unary operator after another operator
                            continue; // Skip for now
                        }
                    }
                    
                    // This is a binary operator
                    return arena_.alloc<BinaryOpNode>(c, 
                        ParseExpression(input.substr(0, i)), 
                        ParseExpression(input.substr(i + 1)));
                }
            }
        }
        return nullptr;
    };

    if (auto node = parse_binary("+-", true)) return node;
    if (auto node = parse_binary("*/", true)) return node;

    // Handle unary operators (after binary parsing fails)
    if (!input.empty() && input.front() == '-') {
        // This is a unary minus
        auto operand = ParseExpression(input.substr(1));
        return arena_.alloc<UnaryOpNode>("u-", operand);
    }
    
    if (!input.empty() && input.front() == '+') {
        // Unary plus (identity operator) - just skip it
        return ParseExpression(input.substr(1));
    }

    // Implicit Mult
    if (input.size() > 1) { 
        int bracket_depth = 0;
        for (size_t i = 0; i < input.size() - 1; ++i) {
            char curr = input[i];
            char next = input[i+1];
            if (curr == '(') bracket_depth++;
            if (curr == ')') bracket_depth--;
            if (bracket_depth == 0) {
                bool digit_alpha = std::isdigit(static_cast<unsigned char>(curr)) && std::isalpha(static_cast<unsigned char>(next));
                bool digit_paren = std::isdigit(static_cast<unsigned char>(curr)) && next == '(';
                bool paren_alpha = (curr == ')') && std::isalpha(static_cast<unsigned char>(next));
                bool paren_paren = (curr == ')') && next == '(';
                
                if (digit_alpha || digit_paren || paren_alpha || paren_paren) {
                    return arena_.alloc<BinaryOpNode>('*', 
                            ParseExpression(input.substr(0, i + 1)), 
                            ParseExpression(input.substr(i + 1)));
                }
            }
        }
    }

    if (auto node = parse_binary("^", false)) return node;

    if (input.size() >= 2 && input.front() == '(' && input.back() == ')') {
        return ParseExpression(input.substr(1, input.size() - 2));
    }

    size_t paren_start = input.find('(');
    if (paren_start != std::string_view::npos && input.back() == ')') {
        auto func_name = input.substr(0, paren_start);
        while(!func_name.empty() && std::isspace(static_cast<unsigned char>(func_name.back()))) func_name.remove_suffix(1);
        auto args_str = input.substr(paren_start + 1, input.size() - paren_start - 2);
        
        // Check if this is a multi-argument function
        if (func_name == "limit" || func_name == "integrate" || func_name == "plot" || 
            func_name == "max" || func_name == "min" || func_name == "gcd" || 
            func_name == "lcm" || func_name == "mod" || func_name == "modulo") {
            std::vector<NodePtr> args;
            
            // Parse comma-separated arguments
            size_t start = 0;
            int paren_depth = 0;
            
            for (size_t i = 0; i <= args_str.size(); ++i) {
                char c = (i < args_str.size()) ? args_str[i] : ','; // Treat end as comma
                
                if (c == '(') paren_depth++;
                else if (c == ')') paren_depth--;
                else if (c == ',' && paren_depth == 0) {
                    // Found argument boundary
                    auto arg_str = args_str.substr(start, i - start);
                    while (!arg_str.empty() && std::isspace(static_cast<unsigned char>(arg_str.front()))) arg_str.remove_prefix(1);
                    while (!arg_str.empty() && std::isspace(static_cast<unsigned char>(arg_str.back()))) arg_str.remove_suffix(1);
                    
                    if (!arg_str.empty()) {
                        args.push_back(ParseExpression(arg_str));
                    }
                    start = i + 1;
                }
            }
            
            return arena_.alloc<MultiArgFunctionNode>(arena_.allocString(func_name), std::move(args));
        } else {
            // Single-argument function (existing behavior)
            return arena_.alloc<UnaryOpNode>(arena_.allocString(func_name), ParseExpression(args_str));
        }
    }
    
    size_t space_pos = input.find(' ');
    if (space_pos != std::string_view::npos) {
        auto func_name = input.substr(0, space_pos);
        auto arg = input.substr(space_pos + 1);
        bool is_func = true;
        for(char c : func_name) if(!std::isalpha(c)) is_func = false;
        if (is_func && !func_name.empty()) {
             return arena_.alloc<UnaryOpNode>(arena_.allocString(func_name), ParseExpression(arg));
        }
    }

    if (Utils::IsNumber(input)) {
        return arena_.alloc<NumberNode>(std::stod(std::string(input)));
    } else {
        if (input.empty()) return arena_.alloc<NumberNode>(0.0);
        return arena_.alloc<VariableNode>(arena_.allocString(input));
    }
}

EngineResult AlgebraicParser::ParseAndExecute(const std::string& input) {
    return ParseAndExecuteWithContext(input, {}); 
}

EngineResult AlgebraicParser::ParseAndExecuteWithContext(const std::string& input, const std::map<std::string, double>& context) {
    // Basic syntax validation
    std::string trimmed = input;
    while (!trimmed.empty() && std::isspace(trimmed.front())) trimmed.erase(0, 1);
    while (!trimmed.empty() && std::isspace(trimmed.back())) trimmed.pop_back();
    
    if (trimmed.empty()) {
        return {{}, {EngineErrorResult(CalcErr::ParseError)}};
    }
    
    // Check for invalid consecutive operators
    for (size_t i = 0; i < trimmed.size() - 1; ++i) {
        char c1 = trimmed[i];
        char c2 = trimmed[i + 1];
        if ((c1 == '+' || c1 == '-' || c1 == '*' || c1 == '/') && 
            (c2 == '+' || c2 == '*' || c2 == '/')) {
            return {{}, {EngineErrorResult(CalcErr::ParseError)}};
        }
    }
    
    // Check for balanced parentheses
    int paren_count = 0;
    for (char c : trimmed) {
        if (c == '(') paren_count++;
        else if (c == ')') paren_count--;
        if (paren_count < 0) {
            return {{}, {EngineErrorResult(CalcErr::ParseError)}};
        }
    }
    if (paren_count != 0) {
        return {{}, {EngineErrorResult(CalcErr::ParseError)}};
    }
    
    // Check for unknown functions (basic validation)
    static const std::set<std::string> known_functions = {
        "sin", "cos", "tan", "asin", "acos", "atan", "sinh", "cosh", "tanh",
        "asinh", "acosh", "atanh", "log", "ln", "log2", "exp", "sqrt", "cbrt",
        "abs", "factorial", "limit", "integrate", "max", "min", "gcd", "lcm",
        "mod", "modulo", "sec", "csc", "cot", "asec", "acsc", "acot",
        "sech", "csch", "coth", "asech", "acsch", "acoth"
    };
    
    // Simple function validation (look for word followed by parentheses)
    size_t pos = 0;
    while ((pos = trimmed.find('(', pos)) != std::string::npos) {
        // Find the start of the potential function name
        size_t func_start = pos;
        while (func_start > 0 && (std::isalpha(trimmed[func_start - 1]) || trimmed[func_start - 1] == '_')) {
            func_start--;
        }
        
        if (func_start < pos) {
            std::string func_name = trimmed.substr(func_start, pos - func_start);
            if (known_functions.find(func_name) == known_functions.end()) {
                return {{}, {EngineErrorResult(CalcErr::ParseError)}};
            }
        }
        pos++;
    }
    
    // Check cache first for performance
    std::string cache_key = input;
    for (const auto& [key, val] : context) {
        cache_key += "_" + key + "=" + std::to_string(val);
    }
    if (eval_cache_.size() < MAX_CACHE_SIZE) {
        auto cache_it = eval_cache_.find(cache_key);
        if (cache_it != eval_cache_.end() && cache_it->second.value.has_value()) {
            return {EngineSuccessResult(*cache_it->second.value), {}};
        } else if (cache_it != eval_cache_.end()) {
            return {{}, {EngineErrorResult(cache_it->second.error)}};
        }
    }
    
    arena_.reset();
    std::string processed_input = input; 
    std::stringstream ss(processed_input);
    std::string first_token;
    ss >> first_token;

    for (const auto& entry : special_commands_) {
        if (first_token == entry.command) {
            auto result = entry.handler(processed_input);
            // Cache the result (only cache double results for now to avoid complexity)
            if (eval_cache_.size() < MAX_CACHE_SIZE) {
                if (result.result.has_value() && std::holds_alternative<double>(*result.result)) {
                    eval_cache_[cache_key] = EvalResult::Success(std::get<double>(*result.result));
                } else if (result.error.has_value() && std::holds_alternative<CalcErr>(*result.error)) {
                    eval_cache_[cache_key] = EvalResult::Failure(std::get<CalcErr>(*result.error));
                }
            }
            return result;
        }
    }

    try {
        NodePtr root = ParseExpression(processed_input);
        auto evaluation = root->Evaluate(context);
        if (evaluation.value.has_value()) {
            // Cache successful evaluation
            if (eval_cache_.size() < MAX_CACHE_SIZE) {
                eval_cache_[cache_key] = evaluation;
            }
            return {EngineSuccessResult(*evaluation.value), {}};
        }
        CalcErr err = evaluation.error == CalcErr::None ? CalcErr::ArgumentMismatch : evaluation.error;
        // Cache error
        if (eval_cache_.size() < MAX_CACHE_SIZE) {
            eval_cache_[cache_key] = evaluation;
        }
        return {{}, {EngineErrorResult(err)}};
    }
    catch (const std::exception&) {
        return {{}, {EngineErrorResult(CalcErr::ArgumentMismatch)}};
    }
}

EngineResult AlgebraicParser::HandleQuadratic(const std::string& input) {
    std::stringstream ss(input);
    std::string cmd;
    double a, b, c;
    ss >> cmd;
    if (!(ss >> a >> b >> c)) return {{}, {EngineErrorResult(CalcErr::ArgumentMismatch)}};
    return SolveQuadratic(a, b, c);
}

EngineResult AlgebraicParser::HandleNonLinearSolve(const std::string& input) {
    auto open_brace = input.find('{');
    auto close_brace = input.find('}');
    if (open_brace == std::string::npos || close_brace == std::string::npos) return {{}, EngineErrorResult(CalcErr::ArgumentMismatch)};
    std::string eq_content = input.substr(open_brace + 1, close_brace - open_brace - 1);
    
    auto open_bracket = input.find('[', close_brace);
    auto close_bracket = input.find(']', open_bracket);
    if (open_bracket == std::string::npos || close_bracket == std::string::npos) return {{}, EngineErrorResult(CalcErr::ArgumentMismatch)};
    std::string guess_content = input.substr(open_bracket + 1, close_bracket - open_bracket - 1);

    auto raw_eqs = Utils::Split(eq_content, ';'); 
    std::vector<std::string> final_equations;
    for (const auto& raw_eq : raw_eqs) {
        std::string eq = Utils::Trim(raw_eq);
        if (eq.empty()) continue;
        size_t eq_sign_pos = eq.find('=');
        if (eq_sign_pos != std::string::npos) {
            std::string lhs = eq.substr(0, eq_sign_pos);
            std::string rhs = eq.substr(eq_sign_pos + 1);
            eq = "(" + lhs + ") - (" + rhs + ")";
        }
        final_equations.push_back(eq);
    }

    auto raw_guesses = Utils::Split(guess_content, ',');
    std::vector<double> guess_values;
    for (const auto& val_str : raw_guesses) {
        std::string trimmed = Utils::Trim(val_str);
        if(Utils::IsNumber(trimmed)) guess_values.push_back(std::stod(trimmed));
    }

    std::map<std::string, double> guess_map;
    std::vector<std::string> var_names = {"x", "y", "z", "a", "b", "c"};
    for(size_t i=0; i<guess_values.size(); ++i) {
        if(i < var_names.size()) guess_map[var_names[i]] = guess_values[i];
    }

    return SolveNonLinearSystem(final_equations, guess_map);
}

EngineResult AlgebraicParser::HandleDerivative(const std::string& input) {
    std::stringstream ss(input);
    std::string cmd, expression, var;
    ss >> cmd;
    std::getline(ss, expression);
    expression = Utils::Trim(expression);
    if (!expression.empty() && expression.back() == ';') expression.pop_back();
    var = "x"; 
    try {
        NodePtr root = ParseExpression(expression);
        NodePtr derivative = root->Derivative(arena_, var);
        NodePtr simplified = derivative->Simplify(arena_)->Simplify(arena_);
        return {EngineSuccessResult(simplified->ToString(Precedence::None)), {}}; 
    } catch (...) {
        return {{}, {EngineErrorResult(CalcErr::ParseError)}};
    }
}

EngineResult AlgebraicParser::SolveQuadratic(double a, double b, double c) {
    if (a == 0.0) return {{}, {EngineErrorResult(CalcErr::IndeterminateResult)}};
    double d = b * b - 4 * a * c;
    if (d < 0) return {{}, {EngineErrorResult(CalcErr::NegativeRoot)}};
    double s = std::sqrt(d);
    return {EngineSuccessResult(Vector({(-b + s) / (2 * a), (-b - s) / (2 * a)})), {}};
}

EngineResult AlgebraicParser::SolveNonLinearSystem(const std::vector<std::string>& equation_strs, std::map<std::string, double>& guess) {
    const int max_iter = 50;
    const double epsilon = 1e-5;
    std::vector<NodePtr> roots;
    for(const auto& eq : equation_strs) roots.push_back(ParseExpression(eq));
    std::vector<std::string> var_names;
    for(auto const& [key, val] : guess) var_names.push_back(key);
    int n = var_names.size();
    for (int iter = 0; iter < max_iter; ++iter) {
        std::vector<double> F(n);
        for(int i=0; i<n; ++i) {
            auto eval = roots[i]->Evaluate(guess);
            if (!eval.value.has_value()) {
                return {{}, EngineErrorResult(NormalizeError(eval, CalcErr::DomainError))};
            }
            F[i] = *eval.value;
        }
        double err = 0; for(double v:F) err+=v*v;
        if(std::sqrt(err) < 1e-6) break;
        std::vector<std::vector<double>> J(n, std::vector<double>(n));
        for (int j = 0; j < n; ++j) {
            std::string v = var_names[j];
            double old = guess[v];
            guess[v] += epsilon;
            for (int i = 0; i < n; ++i) {
                auto eval = roots[i]->Evaluate(guess);
                if (!eval.value.has_value()) {
                    return {{}, EngineErrorResult(NormalizeError(eval, CalcErr::DomainError))};
                }
                J[i][j] = (*eval.value - F[i]) / epsilon;
            }
            guess[v] = old;
        }
        std::vector<double> neg_F = F;
        for(double& val : neg_F) val = -val;
        auto SolveLinearSystemSmall = [](std::vector<std::vector<double>> A, std::vector<double> b) {
            int n = A.size();
            for (int i=0; i<n; ++i) {
                int p=i; for(int k=i+1; k<n; ++k) if(std::abs(A[k][i]) > std::abs(A[p][i])) p=k;
                std::swap(A[i], A[p]); std::swap(b[i], b[p]);
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
        };
        std::vector<double> d = SolveLinearSystemSmall(J, neg_F);
        for(int i=0; i<n; ++i) guess[var_names[i]] += d[i];
    }
    std::vector<double> res;
    for(auto& name : var_names) res.push_back(guess[name]);
    return {EngineSuccessResult(res), {}};
}

EngineResult AlgebraicParser::HandlePlotFunction(const std::string& input) {
    // Parse plot(expression, x_min, x_max, y_min, y_max)
    size_t paren_start = input.find('(');
    size_t paren_end = input.rfind(')');
    
    if (paren_start == std::string::npos || paren_end == std::string::npos) {
        return {{}, {EngineErrorResult(CalcErr::ArgumentMismatch)}};
    }
    
    std::string args_str = input.substr(paren_start + 1, paren_end - paren_start - 1);
    
    // Split arguments by comma (handling nested parentheses)
    std::vector<std::string> args;
    size_t start = 0;
    int paren_depth = 0;
    
    for (size_t i = 0; i <= args_str.size(); ++i) {
        char c = (i < args_str.size()) ? args_str[i] : ',';
        
        if (c == '(') paren_depth++;
        else if (c == ')') paren_depth--;
        else if (c == ',' && paren_depth == 0) {
            std::string arg = Utils::Trim(args_str.substr(start, i - start));
            if (!arg.empty()) {
                args.push_back(arg);
            }
            start = i + 1;
        }
    }
    
    if (args.size() != 5) {
        return {{}, {EngineErrorResult(CalcErr::ArgumentMismatch)}};
    }
    
    // For now, return a special string result to indicate this is a plot command
    // The actual plotting will be handled by the CalcEngine
    std::string plot_command = "PLOT_FUNCTION:" + args[0] + "," + args[1] + "," + args[2] + "," + args[3] + "," + args[4];
    return {EngineSuccessResult(plot_command), {}};
}