#include "pattern.h"

Pattern::Pattern(const std::string& pattern)
    : pattern_(pattern), tokens_(parsePattern(pattern)) {}

bool Pattern::match(const std::string& input) const {
    return matchRecursive(0, input, 0);
}

int Pattern::specificity() const {
    int starCount = 0;
    for (const auto& t : tokens_) {
        if (t.type == TokenType::AnySequence) {
            ++starCount;
        }
    }
    return (starCount == 0 ? 10000 : 0) + static_cast<int>(pattern_.size());
}

std::vector<Pattern::Token> Pattern::parsePattern(const std::string& pattern) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < pattern.size(); ++i) {
        char c = pattern[i];
        if (c == '.') {
            tokens.push_back(Token::Any());
        } else if (c == '*') {
            tokens.push_back(Token::Seq());
        } else if (c == '[') {
            bool negate = false;
            size_t pos = i + 1;
            if (pos < pattern.size() && pattern[pos] == '^') {
                negate = true;
                ++pos;
            }
            std::vector<CharRange> ranges;
            bool have_prev = false;
            char prev_char = 0;

            while (pos < pattern.size() && pattern[pos] != ']') {
                char ch = pattern[pos];
                if (ch == '-' && have_prev && pos + 1 < pattern.size() && pattern[pos + 1] != ']') {
                    char start = prev_char;
                    char end = pattern[pos + 1];
                    ranges.emplace_back(start, end);
                    have_prev = false;
                    pos += 2;
                } else {
                    if (have_prev) {
                        ranges.emplace_back(prev_char, prev_char);
                    }
                    prev_char = ch;
                    have_prev = true;
                    ++pos;
                }
            }
            if (pos >= pattern.size() || pattern[pos] != ']') {
                throw std::runtime_error("Unclosed character class in pattern: " + pattern);
            }
            if (have_prev) {
                ranges.emplace_back(prev_char, prev_char);
            }
            tokens.push_back(Token::Class(std::move(ranges), negate));
            i = pos;
        } else {
            tokens.push_back(Token(c));
        }
    }
    return tokens;
}

bool Pattern::matchRecursive(size_t tokenIdx, const std::string& input, size_t inputIdx) const {
    if (tokenIdx == tokens_.size()) {
        return inputIdx == input.size();
    }

    const Token& tok = tokens_[tokenIdx];
    switch (tok.type) {
        case TokenType::Literal: {
            if (inputIdx < input.size() && input[inputIdx] == tok.literal) {
                return matchRecursive(tokenIdx + 1, input, inputIdx + 1);
            }
            return false;
        }
        case TokenType::AnyChar: {
            if (inputIdx < input.size()) {
                return matchRecursive(tokenIdx + 1, input, inputIdx + 1);
            }
            return false;
        }
        case TokenType::AnySequence: {
            if (matchRecursive(tokenIdx + 1, input, inputIdx)) {
                return true;
            }
            for (size_t i = inputIdx; i < input.size(); ++i) {
                if (matchRecursive(tokenIdx + 1, input, i + 1)) {
                    return true;
                }
            }
            return false;
        }
        case TokenType::CharClass: {
            if (inputIdx >= input.size()) return false;
            char c = input[inputIdx];
            bool matched = false;
            for (const auto& range : tok.ranges) {
                if (range.matches(c)) {
                    matched = true;
                    break;
                }
            }
            if (tok.negated) matched = !matched;
            return matched && matchRecursive(tokenIdx + 1, input, inputIdx + 1);
        }
    }
    return false;
}
