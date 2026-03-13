#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

Lexer::Lexer(const std::string &source) : src(source), pos(0), line(1), col(1) {}

char Lexer::cur() const {
    return pos < src.size() ? src[pos] : '\0';
}

void Lexer::adv() {
    if (pos < src.size()) {
        if (src[pos] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        pos++;
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(src.size() / 2 + 1);

    while (cur()) {
        int tokLine = line;
        int tokCol = col;
        char ch = cur();

        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (ch == '\n') {
                tokens.push_back({TokenType::NEWLINE, "\\n", tokLine, tokCol});
            }
            adv();
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            std::string num;
            num.reserve(16);
            while (std::isdigit(static_cast<unsigned char>(cur()))) {
                num += cur();
                adv();
            }
            tokens.push_back({TokenType::NUMBER, num, tokLine, tokCol});
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch))) {
            std::string id;
            id.reserve(16);
            while (std::isalnum(static_cast<unsigned char>(cur()))) {
                id += cur();
                adv();
            }

            TokenType type = TokenType::IDENT;
            if (id == "let") {
                type = TokenType::LET;
            } else if (id == "const") {
                type = TokenType::CONST;
            } else if (id == "out") {
                type = TokenType::OUT;
            } else if (id == "if") {
                type = TokenType::IF;
            } else if (id == "else") {
                type = TokenType::ELSE;
            } else if (id == "repeat") {
                type = TokenType::REPEAT;
            } else if (id == "times") {
                type = TokenType::TIMES;
            }

            tokens.push_back({type, id, tokLine, tokCol});
            continue;
        }

        if (ch == '"' || ch == '\'') {
            char q = ch;
            adv();
            std::string s;
            while (cur() && cur() != q) {
                s += cur();
                adv();
            }
            adv();
            tokens.push_back({TokenType::STRING, s, tokLine, tokCol});
            continue;
        }

        switch (ch) {
            case '+':
                tokens.push_back({TokenType::PLUS, "+", tokLine, tokCol});
                break;
            case '-':
                tokens.push_back({TokenType::MINUS, "-", tokLine, tokCol});
                break;
            case '*':
                tokens.push_back({TokenType::STAR, "*", tokLine, tokCol});
                break;
            case '/':
                tokens.push_back({TokenType::SLASH, "/", tokLine, tokCol});
                break;
            case '>':
                tokens.push_back({TokenType::GT, ">", tokLine, tokCol});
                break;
            case '<':
                tokens.push_back({TokenType::LT, "<", tokLine, tokCol});
                break;
            case '=':
                adv();
                if (cur() == '=') {
                    tokens.push_back({TokenType::EQEQ, "==", tokLine, tokCol});
                    adv();
                } else {
                    tokens.push_back({TokenType::EQ, "=", tokLine, tokCol});
                }
                continue;
            case '(':
                tokens.push_back({TokenType::LPAREN, "(", tokLine, tokCol});
                break;
            case ')':
                tokens.push_back({TokenType::RPAREN, ")", tokLine, tokCol});
                break;
            case '{':
                tokens.push_back({TokenType::LBRACE, "{", tokLine, tokCol});
                break;
            case '}':
                tokens.push_back({TokenType::RBRACE, "}", tokLine, tokCol});
                break;
            case ';':
                tokens.push_back({TokenType::SEMICOLON, ";", tokLine, tokCol});
                break;
            default:
                std::cout << "Unknown character at line " << tokLine << std::endl;
                std::exit(1);
        }
        adv();
    }

    tokens.push_back({TokenType::END, "", line, col});
    return tokens;
}
