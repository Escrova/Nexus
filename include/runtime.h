#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser.h"
#include "value.h"

class Runtime {
public:
    std::string source;
    std::string sourceName = "<stdin>";

    void emit(const std::string &s);
    void emit(int v);
    void flush() const;

    [[noreturn]] void reportError(int line, int col, const std::string &msg) const;
    [[noreturn]] void runtimeError(int line, int col, const std::string &msg) const;

    void execute(const std::vector<std::unique_ptr<Stmt>> &program);

private:
    struct VariableSlot {
        Value value;
        bool isConst;
    };

    std::vector<std::string> buffer;
    std::vector<std::unordered_map<std::string, VariableSlot>> scopes;

    std::string getLineText(int line) const;
    void printCaretLine(const std::string &codeLine, int col) const;

    void executeStmt(const Stmt *stmt);
    void executeBlock(const BlockStmt *block);
    void executeStatements(const std::vector<std::unique_ptr<Stmt>> &statements);

    void pushScope();
    void popScope();

    void defineVar(const std::string &name, Value value, bool isConst);
    VariableSlot *findVar(const std::string &name);
    const VariableSlot *findVar(const std::string &name) const;

    int evalExpr(const Expr *expr);
};
