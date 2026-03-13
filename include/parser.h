#pragma once

#include <memory>
#include <string>
#include <vector>

#include "token.h"

struct Expr {
    virtual ~Expr() = default;
};

struct NumberExpr final : Expr {
    int value;
    explicit NumberExpr(int value);
};

struct VariableExpr final : Expr {
    std::string name;
    int line;
    int col;

    VariableExpr(std::string name, int line, int col);
};

struct BinaryExpr final : Expr {
    TokenType op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct LetStmt final : Stmt {
    std::string name;
    std::unique_ptr<Expr> initializer;

    LetStmt(std::string name, std::unique_ptr<Expr> initializer);
};

struct ConstStmt final : Stmt {
    std::string name;
    std::unique_ptr<Expr> initializer;

    ConstStmt(std::string name, std::unique_ptr<Expr> initializer);
};

struct OutStmt final : Stmt {
    bool stringLiteral;
    std::string text;
    std::unique_ptr<Expr> expression;

    explicit OutStmt(std::string text);
    explicit OutStmt(std::unique_ptr<Expr> expression);
};

struct BlockStmt final : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements);
};

struct IfStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> thenBlock;
    std::unique_ptr<Stmt> elseBranch;

    IfStmt(std::unique_ptr<Expr> condition,
           std::unique_ptr<BlockStmt> thenBlock,
           std::unique_ptr<Stmt> elseBranch);
};

struct RepeatStmt final : Stmt {
    std::string iterator;
    std::unique_ptr<Expr> countExpr;
    std::unique_ptr<BlockStmt> body;

    RepeatStmt(std::string iterator, std::unique_ptr<Expr> countExpr, std::unique_ptr<BlockStmt> body);
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<std::unique_ptr<Stmt>> parseProgram();

private:
    std::vector<Token> t;
    std::size_t p;

    Token &peek();
    Token &prev();
    bool atEnd() const;
    bool match(TokenType type);
    void skipSeparators();

    Token consume(TokenType type, const std::string &msg);
    [[noreturn]] void syntaxError(const Token &token, const std::string &msg) const;

    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> repeatStatement();
    std::unique_ptr<BlockStmt> block();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> multiplication();
    std::unique_ptr<Expr> primary();
};
