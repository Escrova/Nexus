#include "runtime.h"

#include <cstdlib>
#include <iostream>

void Runtime::emit(const std::string &s) { buffer.push_back(s); }
void Runtime::emit(int v) { buffer.push_back(std::to_string(v)); }

void Runtime::flush() const {
    for (const auto &o : buffer) {
        std::cout << o << std::endl;
    }
}

std::string Runtime::getLineText(int line) const {
    if (line <= 0) {
        return "";
    }

    int currentLine = 1;
    std::size_t start = 0;
    while (currentLine < line && start < source.size()) {
        if (source[start] == '\n') {
            currentLine++;
        }
        start++;
    }

    if (currentLine != line) {
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

void Runtime::printCaretLine(const std::string &codeLine, int col) const {
    for (int i = 1; i < col; ++i) {
        if (i - 1 < static_cast<int>(codeLine.size()) && codeLine[i - 1] == '\t') {
            std::cout << "\t";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "^" << std::endl;
}

[[noreturn]] void Runtime::reportError(int line, int col, const std::string &msg) const {
    std::string codeLine = getLineText(line);
    std::cout << sourceName << ":" << line << ":" << col << ": error: " << msg << std::endl;
    std::cout << codeLine << std::endl;
    printCaretLine(codeLine, col);
    std::exit(1);
}

[[noreturn]] void Runtime::runtimeError(int line, int col, const std::string &msg) const {
    std::string codeLine = getLineText(line);
    std::cout << sourceName << ":" << line << ":" << col << ": runtime error: " << msg << std::endl;
    std::cout << codeLine << std::endl;
    printCaretLine(codeLine, col);
    std::exit(1);
}
    std::exit(1);
}
    std::exit(1);
}
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (currentLine < line && start < source.size()) {
        if (source[start] == '\n') {
            currentLine++;
        }
        start++;
    }

    if (currentLine != line) {
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

void Runtime::printCaretLine(const std::string &codeLine, int col) const {
    std::cout << "Runtime Error (line " << line << ", col " << col << "):" << std::endl;
    std::cout << codeLine << std::endl;
    for (int i = 1; i < col; ++i) {
        if (i - 1 < static_cast<int>(codeLine.size()) && codeLine[i - 1] == '\t') {
            std::cout << "\t";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "^" << std::endl;
}

[[noreturn]] void Runtime::reportError(int line, int col, const std::string &msg) const {
    std::string codeLine = getLineText(line);
    std::cout << sourceName << ":" << line << ":" << col << ": error: " << msg << std::endl;
    std::cout << codeLine << std::endl;
    printCaretLine(codeLine, col);
    std::exit(1);
}

[[noreturn]] void Runtime::runtimeError(int line, int col, const std::string &msg) const {
    std::string codeLine = getLineText(line);
    std::cout << sourceName << ":" << line << ":" << col << ": runtime error: " << msg << std::endl;
    std::cout << codeLine << std::endl;
    printCaretLine(codeLine, col);
    std::exit(1);
}
void Runtime::execute(const std::vector<std::unique_ptr<Stmt>> &program) {
    for (const auto &stmt : program) {
        executeStmt(stmt.get());
    }
}

void Runtime::executeStmt(const Stmt *stmt) {
    switch (stmt->kind) {
        case StmtKind::Let: {
            const auto *letStmt = static_cast<const LetStmt *>(stmt);
            if (letStmt->initializer) {
                vars[letStmt->name] = Value(true, evalExpr(letStmt->initializer.get()));
            } else {
                vars[letStmt->name] = Value();
            }
            isConst[letStmt->name] = false;
            return;
        }
        case StmtKind::Const: {
            const auto *constStmt = static_cast<const ConstStmt *>(stmt);
            vars[constStmt->name] = Value(true, evalExpr(constStmt->initializer.get()));
            isConst[constStmt->name] = true;
            return;
        }
        case StmtKind::Out: {
            const auto *outStmt = static_cast<const OutStmt *>(stmt);
            if (outStmt->stringLiteral) {
                emit(outStmt->text);
            } else {
                emit(evalExpr(outStmt->expression.get()));
            }
            return;
        }
        case StmtKind::Repeat: {
            const auto *repeatStmt = static_cast<const RepeatStmt *>(stmt);
            int count = evalExpr(repeatStmt->countExpr.get());
            for (int i = 0; i < count; ++i) {
                vars[repeatStmt->iterator] = Value(true, i);
                isConst[repeatStmt->iterator] = false;
                executeBlock(repeatStmt->body.get());
            }
            return;
        }
        case StmtKind::If: {
            const auto *ifStmt = static_cast<const IfStmt *>(stmt);
            if (evalExpr(ifStmt->condition.get()) != 0) {
                executeBlock(ifStmt->thenBlock.get());
            } else if (ifStmt->elseBranch) {
                executeStmt(ifStmt->elseBranch.get());
            }
            return;
        }
        case StmtKind::Block: {
            executeBlock(static_cast<const BlockStmt *>(stmt));
            return;
        }
    }
}

void Runtime::executeBlock(const BlockStmt *block) {
    for (const auto &stmt : block->statements) {
        executeStmt(stmt.get());
    }
}

int Runtime::evalExpr(const Expr *expr) {
    switch (expr->kind) {
        case ExprKind::Number:
            return static_cast<const NumberExpr *>(expr)->value;

        case ExprKind::Variable: {
            const auto *variable = static_cast<const VariableExpr *>(expr);
            auto it = vars.find(variable->name);
            if (it == vars.end() || !it->second.initialized) {
                runtimeError(variable->line, variable->col, "use of uninitialized variable '" + variable->name + "'");
            }
            return it->second.number;
        }

        case ExprKind::Binary: {
            const auto *binary = static_cast<const BinaryExpr *>(expr);
            int left = evalExpr(binary->left.get());
            int right = evalExpr(binary->right.get());

            switch (binary->op) {
                case TokenType::PLUS:
                    return left + right;
                case TokenType::MINUS:
                    return left - right;
                case TokenType::STAR:
                    return left * right;
                case TokenType::SLASH:
                    if (right == 0) {
                        runtimeError(binary->line, binary->col, "division by zero");
                    }
                    return left / right;
                case TokenType::GT:
                    return left > right ? 1 : 0;
                case TokenType::LT:
                    return left < right ? 1 : 0;
                case TokenType::EQEQ:
                    return left == right ? 1 : 0;
                default:
                    runtimeError(binary->line, binary->col, "invalid binary operator");
            }
            break;
        }
    }

    runtimeError(0, 0, "invalid expression at runtime");
    return 0;
}
