#include "dictionary.h"
#include <iostream>

bool Dictionary::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    entries_.clear();
    std::string line;
    size_t lineNum = 0;
    while (std::getline(file, line)) {
        ++lineNum;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#') {
            continue;
        }
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            std::cerr << "Warning: line " << lineNum << " has no '=' separator, skipped." << std::endl;
            continue;
        }
        std::string expect = utils::trim(line.substr(0, eqPos));
        std::string answerRaw = line.substr(eqPos + 1);
        std::string answer = utils::unescape(answerRaw);

        expect = utils::trim(expect);

        if (expect.empty()) {
            std::cerr << "Warning: empty expect on line " << lineNum << ", skipped." << std::endl;
            continue;
        }

        entries_.emplace_back(expect, answer);
    }

    std::stable_sort(entries_.begin(), entries_.end(),
                     [](const DictionaryEntry& a, const DictionaryEntry& b) {
                         return a.pattern.specificity() > b.pattern.specificity();
                     });

    return true;
}

bool Dictionary::find(const std::string& command, std::string& answer) const {
    for (const auto& entry : entries_) {
        if (entry.pattern.match(command)) {
            answer = entry.answer;
            return true;
        }
    }
    return false;
}
