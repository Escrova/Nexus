#pragma once

#include <string>

enum class TokenType {
    IDENT,
    NUMBER,
    STRING,
    LET,
    CONST,
    OUT,
    IF,
    ELSE,
    REPEAT,
    TIMES,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    GT,
    LT,
    EQ,
    EQEQ,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    NEWLINE,
    END
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
};
