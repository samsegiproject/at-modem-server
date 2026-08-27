#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "pattern.h"
#include "utils.h"

struct DictionaryEntry {
    Pattern pattern;
    std::string answer;
    std::string originalPattern;

    DictionaryEntry(const std::string& pat, const std::string& ans)
        : pattern(pat), answer(ans), originalPattern(pat) {}
};

class Dictionary {
public:
    bool loadFromFile(const std::string& filename);
    bool find(const std::string& command, std::string& answer) const;
    const std::vector<DictionaryEntry>& entries() const { return entries_; }

private:
    std::vector<DictionaryEntry> entries_;
};
