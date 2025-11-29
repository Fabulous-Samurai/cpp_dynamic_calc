/**
 * @file algebraic_parser.cpp
 * @brief Implementation of the Algebraic Parser using AST.
 * RESTORED: Full functionality including Evaluate, Derivative, and Simplify logic.
 */

#include "algebraic_parser.h"
#include "string_helpers.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <set>

// --- Helpers ---

Precedence GetOpPrecedence(char op) {
    if (op == '+' || op == '-') return Precedence::AddSub;
    if (op == '*' || op == '/') return Precedence::MultiDiv;
    if (op == '^') return Precedence::Pow;
    return Precedence::None;
}

// Bu fonksiyon Evaluate'i kullanır! Silinirse sadeleştirme bozulur.
bool IsConst(const NodePtr node, double val) {
    try { return std::abs(node->Evaluate({}) - val) < 1e-9; } catch (...) { return false; }
}

std::string FormatNumber(double val) {
    std::string s = std::to_string(val);
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.') s.pop_back();
    return s;
}

// ========================================================
// AST NODE IMPLEMENTATIONS
// ========================================================

struct NumberNode : ExprNode {
    double value;
    NumberNode(double v) : value(v) {}
    
    // Evaluate SİLİNMEMELİ! Hesaplama burada yapılır.
    double Evaluate(const std::map<std::string, double>&) const override { return value; }
    
    NodePtr Derivative(Arena& arena, std::string_view) const override { return arena.alloc<NumberNode>(0.0); }
    NodePtr Simplify(Arena& arena) const override { return arena.alloc<NumberNode>(value); }
    std::string ToString(Precedence) const override { return FormatNumber(value); }
};

struct VariableNode : ExprNode {
    std::string_view name;
    VariableNode(std::string_view n) : name(n) {}
    
    double Evaluate(const std::map<std::string, double>& vars) const override {
        std::string key(name);
        if (vars.count(key)) return vars.at(key);
        if (key == "Ans") return 0.0;
        throw std::runtime_error("Undefined variable: " + key);
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
    double Evaluate(const std::map<std::string, double>& vars) const override {
        double l = left->Evaluate(vars);
        double r = right->Evaluate(vars);
        switch(op) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/': return (r != 0.0) ? l / r : throw std::runtime_error("Division by zero");
            case '^': return std::pow(l, r);
            default: return 0.0;
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

        // Constant Folding (Evaluate burada kullanılıyor!)
        bool l_const = false, r_const = false;
        double l_val = 0, r_val = 0;
        try { l_val = simple_left->Evaluate({}); l_const = true; } catch(...) {}
        try { r_val = simple_right->Evaluate({}); r_const = true; } catch(...) {}

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
    
    // [KRİTİK] Burayı da silme! Fonksiyonların sayısal hesabı buradadır.
    double Evaluate(const std::map<std::string, double>& vars) const override {
        double val = operand->Evaluate(vars);
        if (func == "sin") return std::sin(val * D2R);
        if (func == "cos") return std::cos(val * D2R);
        if (func == "tan") return std::tan(val * D2R);
        if (func == "cot") return 1.0 / std::tan(val * D2R);
        if (func == "sec") return 1.0 / std::cos(val * D2R);
        if (func == "csc") return 1.0 / std::sin(val * D2R);
        
        if (func == "asin") return std::asin(val) * R2D;
        if (func == "acos") return std::acos(val) * R2D;
        if (func == "atan") return std::atan(val) * R2D;
        
        if (func == "sinh") return std::sinh(val);
        if (func == "cosh") return std::cosh(val);
        if (func == "tanh") return std::tanh(val);
        
        if (func == "sqrt") { if (val < 0) throw std::runtime_error("Negative sqrt"); return std::sqrt(val); }
        if (func == "cbrt") return std::cbrt(val);
        if (func == "abs") return std::abs(val);
        if (func == "ln") { if (val <= 0) throw std::runtime_error("Log domain error"); return std::log(val); }
        if (func == "log") { if (val <= 0) throw std::runtime_error("Log domain error"); return std::log10(val); }
        if (func == "log2" || func == "lg") { if (val <= 0) throw std::runtime_error("Log domain error"); return std::log2(val); }
        if (func == "exp") return std::exp(val);
        
        if (func == "u-") return -val;
        return 0.0;
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
        auto inner = input.substr(paren_start + 1, input.size() - paren_start - 2);
        return arena_.alloc<UnaryOpNode>(arena_.allocString(func_name), ParseExpression(inner));
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
    arena_.reset();
    std::string processed_input = input; 
    std::stringstream ss(processed_input);
    std::string first_token;
    ss >> first_token;

    for (const auto& entry : special_commands_) {
        if (first_token == entry.command) {
            return entry.handler(processed_input);
        }
    }

    try {
        NodePtr root = ParseExpression(processed_input);
        double result = root->Evaluate(context); 
        return {EngineSuccessResult(result), {}};
    } 
    catch (const std::exception& e) {
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
            try { F[i] = roots[i]->Evaluate(guess); } 
            catch (...) { return {{}, EngineErrorResult(CalcErr::DomainError)}; }
        }
        double err = 0; for(double v:F) err+=v*v;
        if(std::sqrt(err) < 1e-6) break;
        std::vector<std::vector<double>> J(n, std::vector<double>(n));
        for (int j = 0; j < n; ++j) {
            std::string v = var_names[j];
            double old = guess[v];
            guess[v] += epsilon;
            for (int i = 0; i < n; ++i) J[i][j] = (roots[i]->Evaluate(guess) - F[i]) / epsilon;
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