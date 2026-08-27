#pragma once

#include <string>
#include <vector>
#include <stdexcept>

class Pattern {
public:
    enum class TokenType {
        Literal,
        AnyChar,      // .
        AnySequence,  // *
        CharClass     // [...]
    };

    struct CharRange {
        char start;
        char end;
        CharRange(char s, char e) : start(s), end(e) {}
        bool matches(char c) const { return c >= start && c <= end; }
    };

    struct Token {
        TokenType type;
        char literal;
        std::vector<CharRange> ranges;
        bool negated;

        Token(TokenType t) : type(t), literal(0), negated(false) {}
        Token(char c) : type(TokenType::Literal), literal(c), negated(false) {}
        static Token Any() { return Token(TokenType::AnyChar); }
        static Token Seq() { return Token(TokenType::AnySequence); }
        static Token Class(std::vector<CharRange> r, bool n = false) {
            Token t(TokenType::CharClass);
            t.ranges = std::move(r);
            t.negated = n;
            return t;
        }
    };

    explicit Pattern(const std::string& pattern);
    bool match(const std::string& input) const;
    const std::string& getPattern() const { return pattern_; }
    int specificity() const;

private:
    std::string pattern_;
    std::vector<Token> tokens_;

    bool matchRecursive(size_t tokenIdx, const std::string& input, size_t inputIdx) const;
    static std::vector<Token> parsePattern(const std::string& pattern);
};
