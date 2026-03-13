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
    std::vector<std::string> buffer;
    std::unordered_map<std::string, Value> vars;
    std::unordered_map<std::string, bool> isConst;

    std::string getLineText(int line) const;
    void printCaretLine(const std::string &codeLine, int col) const;

    void executeStmt(const Stmt *stmt);
    void executeBlock(const BlockStmt *block);
    int evalExpr(const Expr *expr);
};
