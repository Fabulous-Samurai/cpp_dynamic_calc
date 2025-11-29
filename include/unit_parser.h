#pragma once
#include "IParser.h"
#include "unit_manager.h"
#include <string>
#include <regex>

class UnitParser : public IParser {
private:
    UnitManager* unit_manager_;
    
public:
    UnitParser(UnitManager* manager) : unit_manager_(manager) {}
    
    EngineResult ParseAndExecute(const std::string& input) override;
    
private:
    bool IsUnitConversion(const std::string& input);
    EngineResult ParseConversion(const std::string& input);
};