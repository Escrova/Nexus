#include "parser.h"

#include <cstdlib>
#include <iostream>
#include <utility>

Expr::Expr(ExprKind kind) : kind(kind) {}
Stmt::Stmt(StmtKind kind) : kind(kind) {}

NumberExpr::NumberExpr(int value) : Expr(ExprKind::Number), value(value) {}
VariableExpr::VariableExpr(std::string name, int line, int col)
    : Expr(ExprKind::Variable), name(std::move(name)), line(line), col(col) {}
BinaryExpr::BinaryExpr(TokenType op, int line, int col, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
    : Expr(ExprKind::Binary), op(op), line(line), col(col), left(std::move(left)), right(std::move(right)) {}

LetStmt::LetStmt(std::string name, std::unique_ptr<Expr> initializer)
    : Stmt(StmtKind::Let), name(std::move(name)), initializer(std::move(initializer)) {}
ConstStmt::ConstStmt(std::string name, std::unique_ptr<Expr> initializer)
    : Stmt(StmtKind::Const), name(std::move(name)), initializer(std::move(initializer)) {}
OutStmt::OutStmt(std::string text)
    : Stmt(StmtKind::Out), stringLiteral(true), text(std::move(text)), expression(nullptr) {}
OutStmt::OutStmt(std::unique_ptr<Expr> expression)
    : Stmt(StmtKind::Out), stringLiteral(false), expression(std::move(expression)) {}
BlockStmt::BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
    : Stmt(StmtKind::Block), statements(std::move(statements)) {}
IfStmt::IfStmt(std::unique_ptr<Expr> condition,
               std::unique_ptr<BlockStmt> thenBlock,
               std::unique_ptr<Stmt> elseBranch)
    : Stmt(StmtKind::If),
      condition(std::move(condition)),
      thenBlock(std::move(thenBlock)),
      elseBranch(std::move(elseBranch)) {}
RepeatStmt::RepeatStmt(std::string iterator, std::unique_ptr<Expr> countExpr, std::unique_ptr<BlockStmt> body)
    : Stmt(StmtKind::Repeat),
      iterator(std::move(iterator)),
      countExpr(std::move(countExpr)),
      body(std::move(body)) {}

Parser::Parser(std::vector<Token> tokens, const std::string &source, const std::string &sourceName)
    : t(std::move(tokens)), source(source), sourceName(sourceName), p(0) {}
Parser::Parser(std::vector<Token> tokens, const std::string &source)
    : Parser(std::move(tokens), source, "<stdin>") {}
Token &Parser::peek() { return t[p]; }
Token &Parser::prev() { return t[p - 1]; }
bool Parser::atEnd() const { return t[p].type == TokenType::END; }

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        p++;
        return true;
    }
    return false;
}

void Parser::skipSeparators() {
    while (match(TokenType::NEWLINE)) {
    }
}

const Token &Parser::consume(TokenType type, const std::string &msg) {
    if (!match(type)) {
        syntaxError(peek(), msg);
    }
    return prev();
}

std::string Parser::getLineText(int targetLine) const {
    if (targetLine <= 0) {
        return "";
    }

    int currentLine = 1;
    std::size_t start = 0;
    while (currentLine < targetLine && start < source.size()) {
        if (source[start] == '\n') {
            currentLine++;
        }
        start++;
    }

    if (currentLine != targetLine) {
        return "";
    }

    std::size_t end = start;
    while (end < source.size() && source[end] != '\n') {
        end++;
    }

    std::string codeLine = source.substr(start, end - start);
    while (!codeLine.empty() && (codeLine.back() == ' ' || codeLine.back() == '\t')) {
        codeLine.pop_back();
    }
    return codeLine;
}

[[noreturn]] void Parser::syntaxError(const Token &token, const std::string &msg) const {
    std::string codeLine = getLineText(token.line);

    std::cout << sourceName << ":" << token.line << ":" << token.col << ": error: " << msg << std::endl;
    std::cout << "Syntax Error (line " << token.line << ", col " << token.col << "):" << std::endl;
    std::cout << codeLine << std::endl;
    for (int i = 1; i < token.col; ++i) {
        if (i - 1 < static_cast<int>(codeLine.size()) && codeLine[i - 1] == '\t') {
            std::cout << "\t";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "^" << std::endl;
    std::cout << msg << std::endl;
    std::exit(1);
}

std::vector<std::unique_ptr<Stmt>> Parser::parseProgram() {
    std::vector<std::unique_ptr<Stmt>> program;
    skipSeparators();
    while (!atEnd()) {
        program.push_back(statement());
        skipSeparators();
    }
    return program;
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::LET)) {
        const Token &nameToken = consume(TokenType::IDENT, "expected variable name");
        std::unique_ptr<Expr> initializer;
        if (match(TokenType::EQ)) {
            initializer = expression();
        }
        consume(TokenType::SEMICOLON, "expected ';'");
        return std::make_unique<LetStmt>(nameToken.value, std::move(initializer));
    }

    if (match(TokenType::CONST)) {
        const Token &nameToken = consume(TokenType::IDENT, "expected variable name");
        consume(TokenType::EQ, "expected '='");
        auto initializer = expression();
        consume(TokenType::SEMICOLON, "expected ';'");
        return std::make_unique<ConstStmt>(nameToken.value, std::move(initializer));
    }

    if (match(TokenType::OUT)) {
        consume(TokenType::LPAREN, "expected '('");
        std::unique_ptr<Stmt> outStmt;
        if (match(TokenType::STRING)) {
            outStmt = std::make_unique<OutStmt>(prev().value);
        } else {
            outStmt = std::make_unique<OutStmt>(expression());
        }
        consume(TokenType::RPAREN, "expected ')'");
        consume(TokenType::SEMICOLON, "expected ';'");
        return outStmt;
    }

    if (match(TokenType::REPEAT)) {
        return repeatStatement();
    }

    if (match(TokenType::IF)) {
        return ifStatement();
    }

    syntaxError(peek(), "unexpected token '" + peek().value + "'");
    return nullptr;
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    auto cond = expression();
    auto thenBlock = block();

    std::unique_ptr<Stmt> elseBranch;
    if (match(TokenType::ELSE)) {
        if (match(TokenType::IF)) {
            elseBranch = ifStatement();
        } else {
            elseBranch = block();
        }
    }

    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::repeatStatement() {
    const Token &it = consume(TokenType::IDENT, "expected iterator");
    auto count = expression();
    consume(TokenType::TIMES, "expected 'times'");
    auto body = block();
    return std::make_unique<RepeatStmt>(it.value, std::move(count), std::move(body));
}

std::unique_ptr<BlockStmt> Parser::block() {
    consume(TokenType::LBRACE, "expected '{'");
    skipSeparators();
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!match(TokenType::RBRACE)) {
        if (atEnd()) {
            syntaxError(peek(), "expected '}'");
        }
        statements.push_back(statement());
        skipSeparators();
    }
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Expr> Parser::expression() { return equality(); }

std::unique_ptr<Expr> Parser::equality() {
    auto left = comparison();
    while (peek().type == TokenType::EQEQ) {
        const Token &op = peek();
        p++;
        auto right = comparison();
        left = std::make_unique<BinaryExpr>(op.type, op.line, op.col, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto left = addition();
    while (peek().type == TokenType::GT || peek().type == TokenType::LT) {
        const Token &op = peek();
        p++;
        auto right = addition();
        left = std::make_unique<BinaryExpr>(op.type, op.line, op.col, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::addition() {
    auto left = multiplication();
    while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
        const Token &op = peek();
        p++;
        auto right = multiplication();
        left = std::make_unique<BinaryExpr>(op.type, op.line, op.col, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::multiplication() {
    auto left = primary();
    while (peek().type == TokenType::STAR || peek().type == TokenType::SLASH) {
        const Token &op = peek();
        p++;
        auto right = primary();
        left = std::make_unique<BinaryExpr>(op.type, op.line, op.col, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::NUMBER)) {
        return std::make_unique<NumberExpr>(std::stoi(prev().value));
    }

    if (match(TokenType::IDENT)) {
        return std::make_unique<VariableExpr>(prev().value, prev().line, prev().col);
    }

    if (match(TokenType::LPAREN)) {
        auto inside = expression();
        consume(TokenType::RPAREN, "expected ')'");
        return inside;
    }

    syntaxError(peek(), "invalid expression");
    return nullptr;
}
