#include <iostream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"

int main() {
    std::string src;
    std::string line;
    while (std::getline(std::cin, line)) {
        src += line + "\n";
    }

    Runtime runtime;
    runtime.source = src;

    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    auto program = parser.parseProgram();

    runtime.execute(program);
    runtime.flush();
    return 0;
}
