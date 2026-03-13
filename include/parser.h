#pragma once

#include <memory>
#include <string>
#include <vector>

#include "token.h"

enum class ExprKind {
    Number,
    Variable,
    Binary
};

struct Expr {
    const ExprKind kind;
    explicit Expr(ExprKind kind);
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
    int line;
    int col;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(TokenType op, int line, int col, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);
};

enum class StmtKind {
    Let,
    Const,
    Out,
    Block,
    If,
    Repeat
};

enum class StmtKind {
    Let,
    Const,
    Out,
    Block,
    If,
    Repeat
};

struct Stmt {
    const StmtKind kind;
    explicit Stmt(StmtKind kind);
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
    Parser(std::vector<Token> tokens, const std::string &source, const std::string &sourceName);
    Parser(std::vector<Token> tokens, const std::string &source);
    std::vector<std::unique_ptr<Stmt>> parseProgram();

private:
    std::vector<Token> t;
    const std::string &source;
    const std::string &sourceName;
    std::size_t p;

    Token &peek();
    Token &prev();
    bool atEnd() const;
    bool match(TokenType type);
    void skipSeparators();

    const Token &consume(TokenType type, const std::string &msg);
    [[noreturn]] void syntaxError(const Token &token, const std::string &msg) const;

    std::string getLineText(int targetLine) const;

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
