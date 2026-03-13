#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

Lexer::Lexer(const std::string &source, const std::string &sourceName)
    : src(source), sourceName(sourceName), pos(0), line(1), col(1) {}

char Lexer::cur() const {
    return pos < src.size() ? src[pos] : '\0';
}

char Lexer::next() const {
    return pos + 1 < src.size() ? src[pos + 1] : '\0';
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

std::string Lexer::getLineText(int targetLine) const {
    if (targetLine <= 0) {
        return "";
    }

    int currentLine = 1;
    std::size_t start = 0;
    while (currentLine < targetLine && start < src.size()) {
        if (src[start] == '\n') {
            currentLine++;
        }
        start++;
    }

    if (currentLine != targetLine) {
        return "";
    }

    std::size_t end = start;
    while (end < src.size() && src[end] != '\n') {
        end++;
    }

    std::string codeLine = src.substr(start, end - start);
    while (!codeLine.empty() && (codeLine.back() == ' ' || codeLine.back() == '\t')) {
        codeLine.pop_back();
    }
    return codeLine;
}

[[noreturn]] void Lexer::reportError(int errorLine, int errorCol, const std::string &msg) const {
    std::string codeLine = getLineText(errorLine);

    std::cout << sourceName << ":" << errorLine << ":" << errorCol << ": error: " << msg << std::endl;
    std::cout << codeLine << std::endl;
    for (int i = 1; i < errorCol; ++i) {
        if (i - 1 < static_cast<int>(codeLine.size()) && codeLine[i - 1] == '\t') {
            std::cout << "\t";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "^" << std::endl;
    std::exit(1);
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

        if (ch == '/' && next() == '/') {
            while (cur() && cur() != '\n') {
                adv();
            }
            continue;
        }

        if (ch == '/' && next() == '*') {
            int commentLine = tokLine;
            int commentCol = tokCol;
            adv();
            adv();

            bool closed = false;
            while (cur()) {
                if (cur() == '*' && next() == '/') {
                    adv();
                    adv();
                    closed = true;
                    break;
                }
                adv();
            }

            if (!closed) {
                reportError(commentLine, commentCol, "unterminated block comment");
            }
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

        if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
            std::string id;
            id.reserve(16);
            while (std::isalnum(static_cast<unsigned char>(cur())) || cur() == '_') {
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
                if (cur() == '\n') {
                    reportError(tokLine, tokCol, "unterminated string literal");
                }
                s += cur();
                adv();
            }
            if (!cur()) {
                reportError(tokLine, tokCol, "unterminated string literal");
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
                reportError(tokLine, tokCol, std::string("unknown character '") + ch + "'");
        }
        adv();
    }

    tokens.push_back({TokenType::END, "", line, col});
    return tokens;
}
