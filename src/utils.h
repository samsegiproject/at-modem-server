#pragma once

#include <string>
#include <algorithm>
#include <cctype>

namespace utils {

// Trim leading and trailing whitespace from a string
inline std::string trim(const std::string& s) {
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();
    return (start < end) ? std::string(start, end) : std::string();
}

// Unescape escape sequences: \r, \n, \\, \=
inline std::string unescape(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case 'r': result += '\r'; ++i; break;
                case 'n': result += '\n'; ++i; break;
                case '\\': result += '\\'; ++i; break;
                case '=': result += '='; ++i; break;
                default: result += s[i];
            }
        } else {
            result += s[i];
        }
    }
    return result;
}

// Check if string ends with newline (\r\n or \n)
inline bool ends_with_newline(const std::string& s) {
    if (s.empty()) return false;
    if (s.back() == '\n') return true;
    if (s.size() >= 2 && s[s.size()-2] == '\r' && s.back() == '\n') return true;
    return false;
}

} // namespace utils
