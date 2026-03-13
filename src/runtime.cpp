#include "runtime.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

void Runtime::emit(const std::string &s) { buffer.push_back(s); }
void Runtime::emit(int v) { buffer.push_back(std::to_string(v)); }

void Runtime::flush() const {
    for (const auto &o : buffer) {
        std::cout << o << std::endl;
    }
}

[[noreturn]] void Runtime::reportError(int line, int col, const std::string &msg) const {
    std::vector<std::string> lines;
    std::size_t start = 0;
    std::size_t end;

    while ((end = source.find('\n', start)) != std::string::npos) {
        lines.push_back(source.substr(start, end - start));
        start = end + 1;
    }
    lines.push_back(source.substr(start));

    std::string codeLine;
    if (line > 0 && static_cast<std::size_t>(line) <= lines.size()) {
        codeLine = lines[line - 1];
    }

    while (!codeLine.empty() && (codeLine.back() == ' ' || codeLine.back() == '\t')) {
        codeLine.pop_back();
    }

    std::cout << "Syntax Error (line " << line << ", col " << col << "):" << std::endl;
    std::cout << codeLine << std::endl;
    for (int i = 1; i < col; ++i) {
        if (i - 1 < static_cast<int>(codeLine.size()) && codeLine[i - 1] == '\t') std::cout << "\t";
        else std::cout << " ";
    }
    std::cout << "^" << std::endl;
    std::cout << msg << std::endl;
    std::exit(1);
}

[[noreturn]] void Runtime::runtimeError(int line, int col, const std::string &msg) const {
    std::cout << "Runtime Error (line " << line << ", col " << col << "):" << std::endl;
    std::cout << msg << std::endl;
    std::exit(1);
}

void Runtime::execute(const std::vector<std::unique_ptr<Stmt>> &program) {
    for (const auto &stmt : program) {
        executeStmt(stmt.get());
    }
}

void Runtime::executeStmt(const Stmt *stmt) {
    if (const auto *letStmt = dynamic_cast<const LetStmt *>(stmt)) {
        if (letStmt->initializer) {
            vars[letStmt->name] = Value(true, evalExpr(letStmt->initializer.get()));
        } else {
            vars[letStmt->name] = Value();
        }
        isConst[letStmt->name] = false;
        return;
    }

    if (const auto *constStmt = dynamic_cast<const ConstStmt *>(stmt)) {
        vars[constStmt->name] = Value(true, evalExpr(constStmt->initializer.get()));
        isConst[constStmt->name] = true;
        return;
    }

    if (const auto *outStmt = dynamic_cast<const OutStmt *>(stmt)) {
        if (outStmt->stringLiteral) emit(outStmt->text);
        else emit(evalExpr(outStmt->expression.get()));
        return;
    }

    if (const auto *repeatStmt = dynamic_cast<const RepeatStmt *>(stmt)) {
        int count = evalExpr(repeatStmt->countExpr.get());
        for (int i = 0; i < count; ++i) {
            vars[repeatStmt->iterator] = Value(true, i);
            isConst[repeatStmt->iterator] = false;
            executeBlock(repeatStmt->body.get());
        }
        return;
    }

    if (const auto *ifStmt = dynamic_cast<const IfStmt *>(stmt)) {
        if (evalExpr(ifStmt->condition.get()) != 0) {
            executeBlock(ifStmt->thenBlock.get());
        } else if (ifStmt->elseBranch) {
            executeStmt(ifStmt->elseBranch.get());
        }
        return;
    }

    if (const auto *blockStmt = dynamic_cast<const BlockStmt *>(stmt)) {
        executeBlock(blockStmt);
        return;
    }
}

void Runtime::executeBlock(const BlockStmt *block) {
    for (const auto &stmt : block->statements) {
        executeStmt(stmt.get());
    }
}

int Runtime::evalExpr(const Expr *expr) {
    if (const auto *number = dynamic_cast<const NumberExpr *>(expr)) {
        return number->value;
    }

    if (const auto *variable = dynamic_cast<const VariableExpr *>(expr)) {
        auto it = vars.find(variable->name);
        if (it == vars.end() || !it->second.initialized) {
            runtimeError(variable->line, variable->col, "use of uninitialized variable '" + variable->name + "'");
        }
        return it->second.number;
    }

    if (const auto *binary = dynamic_cast<const BinaryExpr *>(expr)) {
        int left = evalExpr(binary->left.get());
        int right = evalExpr(binary->right.get());

        switch (binary->op) {
            case TokenType::PLUS: return left + right;
            case TokenType::MINUS: return left - right;
            case TokenType::STAR: return left * right;
            case TokenType::SLASH:
                if (right == 0) runtimeError(0, 0, "division by zero");
                return left / right;
            case TokenType::GT: return left > right ? 1 : 0;
            case TokenType::LT: return left < right ? 1 : 0;
            case TokenType::EQEQ: return left == right ? 1 : 0;
            default: break;
        }
    }

    runtimeError(0, 0, "invalid expression at runtime");
}
