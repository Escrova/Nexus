#pragma once

#include <string>
#include <vector>

#include "token.h"

class Lexer {
public:
    Lexer(const std::string &source, const std::string &sourceName);
    std::vector<Token> tokenize();

private:
    const std::string &src;
    const std::string &sourceName;
    std::size_t pos;
    int line;
    int col;

    char cur() const;
    char next() const;
    void adv();

    std::string getLineText(int targetLine) const;
    [[noreturn]] void reportError(int errorLine, int errorCol, const std::string &msg) const;
};
