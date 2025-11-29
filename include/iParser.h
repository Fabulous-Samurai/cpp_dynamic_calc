#pragma once
#include <string>
#include "dynamic_calc_types.h"
#ifndef CPP_DYNAMIC_CALC_IPARSER_H
#define CPP_DYNAMIC_CALC_IPARSER_H
class IParser {
public:
    virtual ~IParser() = default;
    virtual EngineResult ParseAndExecute(const std::string& input) = 0;
};
#endif //CPP_DYNAMIC_CALC_IPARSER_H
