#pragma once

#include "IParser.h"
#include "dynamic_calc_types.h"
#include <map>
#include <sstream>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <shared_mutex>
class AlgebraicParser : public IParser {
public:
    AlgebraicParser();
    EngineResult ParseAndExecute(const std::string& input) override;

    void RegisterOperator(const std::string& op, const OperatorDetails& details);
    void RegisterUnaryOperator(const std::string& op, const UnaryOperatorDetails& details);

private:
    std::map<std::string, OperatorDetails> ops_;
    std::map<std::string, UnaryOperatorDetails> unary_ops_;
    mutable std::shared_mutex mutex_s;

    bool isNumber(const std::string &token) const;
    Precedence get_precedence(const std::string &token) const;
    bool isLeftAssociative(const std::string &token) const;
    bool isSeparator(char c) const;

    std::queue<std::string> ParseToRPN(const std::string &expression);
    EngineResult EvaluateRPN(std::queue<std::string> &rpn_queue);
};

