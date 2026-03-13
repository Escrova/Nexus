#pragma once

#include <string>
#include <vector>

#include "token.h"

class Lexer {
public:
    explicit Lexer(const std::string &source);
    std::vector<Token> tokenize();

private:
    const std::string &src;
    std::size_t pos;
    int line;
    int col;

    char cur() const;
    void adv();
};
