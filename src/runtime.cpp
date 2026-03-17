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

void Runtime::pushScope() { scopes.emplace_back(); }

void Runtime::popScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void Runtime::defineVar(const std::string &name, Value value, bool isConst) {
    if (scopes.empty()) {
        pushScope();
    }
    scopes.back()[name] = VariableSlot{value, isConst};
}

Runtime::VariableSlot *Runtime::findVar(const std::string &name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

const Runtime::VariableSlot *Runtime::findVar(const std::string &name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

void Runtime::execute(const std::vector<std::unique_ptr<Stmt>> &program) {
    scopes.clear();
    pushScope();
    executeStatements(program);
}

void Runtime::executeStatements(const std::vector<std::unique_ptr<Stmt>> &statements) {
    for (const auto &stmt : statements) {
        executeStmt(stmt.get());
    }
}

void Runtime::executeStmt(const Stmt *stmt) {
    switch (stmt->kind) {
        case StmtKind::Let: {
            const auto *letStmt = static_cast<const LetStmt *>(stmt);
            if (letStmt->initializer) {
                defineVar(letStmt->name, Value(true, evalExpr(letStmt->initializer.get())), false);
            } else {
                defineVar(letStmt->name, Value(), false);
            }
            return;
        }
        case StmtKind::Const: {
            const auto *constStmt = static_cast<const ConstStmt *>(stmt);
            defineVar(constStmt->name, Value(true, evalExpr(constStmt->initializer.get())), true);
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
        case StmtKind::Assign: {
            const auto *assignStmt = static_cast<const AssignStmt *>(stmt);
            VariableSlot *slot = findVar(assignStmt->name);
            if (slot == nullptr) {
                runtimeError(assignStmt->line, assignStmt->col, "assignment to undefined variable '" + assignStmt->name + "'");
            }
            if (slot->isConst) {
                runtimeError(assignStmt->line, assignStmt->col, "cannot assign to const variable '" + assignStmt->name + "'");
            }
            slot->value = Value(true, evalExpr(assignStmt->value.get()));
            return;
        }
        case StmtKind::Repeat: {
            const auto *repeatStmt = static_cast<const RepeatStmt *>(stmt);
            int count = evalExpr(repeatStmt->countExpr.get());
            for (int i = 0; i < count; ++i) {
                pushScope();
                defineVar(repeatStmt->iterator, Value(true, i), false);
                executeStatements(repeatStmt->body->statements);
                popScope();
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
    pushScope();
    executeStatements(block->statements);
    popScope();
}

int Runtime::evalExpr(const Expr *expr) {
    switch (expr->kind) {
        case ExprKind::Number:
            return static_cast<const NumberExpr *>(expr)->value;

        case ExprKind::Variable: {
            const auto *variable = static_cast<const VariableExpr *>(expr);
            const VariableSlot *slot = findVar(variable->name);
            if (slot == nullptr || !slot->value.initialized) {
                runtimeError(variable->line, variable->col, "use of uninitialized variable '" + variable->name + "'");
            }
            return slot->value.number;
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
