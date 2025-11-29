#pragma once
#include <string>
#include <string_view>
#include <charconv>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Utils {

    // Helper to trim strings (removes whitespace from both ends)
    inline std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (std::string::npos == first) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }
    
    // Helper to split string by delimiter
    inline std::vector<std::string> Split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            // Split ederken otomatik Trim yapıyoruz ki " x " gibi durumlar oluşmasın
            std::string trimmed = Trim(token);
            if(!trimmed.empty()) {
                tokens.push_back(trimmed);
            }
        }
        return tokens;
    }

    // Modern C++ Way: Exception-free number check.
    inline bool IsNumber(std::string_view str) {
        if (str.empty()) return false;

        // Trim temporary view for check
        size_t first = str.find_first_not_of(' ');
        if (std::string_view::npos == first) return false;
        str.remove_prefix(first);
        size_t last = str.find_last_not_of(' ');
        if (std::string_view::npos != last) str = str.substr(0, last + 1);

        if (str.empty()) return false;
        
        const char* first_ptr = str.data();
        const char* last_ptr = str.data() + str.size();
        double result;
        auto [ptr, ec] = std::from_chars(first_ptr, last_ptr, result);
        return (ec == std::errc()) && (ptr == last_ptr);
    }
    
    // Helper for ReplaceAns logic (Moved from main.cpp)
    inline std::string ReplaceAns(std::string input, double last_val) {
        const std::string search = "Ans";
        size_t pos = 0;
        if (input.find(search) == std::string::npos) return input;

        std::stringstream ss;
        ss.precision(15);
        ss << last_val;
        std::string replace = ss.str();

        while ((pos = input.find(search, pos)) != std::string::npos) {
            input.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return input;
    }
}