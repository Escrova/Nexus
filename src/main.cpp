#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"

int main(int argc, char **argv) {
    std::string src;
    std::string sourceName = "<stdin>";

    if (argc > 1) {
        std::ifstream inputFile(argv[1]);
        if (!inputFile) {
            std::cerr << "Failed to open source file: " << argv[1] << std::endl;
            return 1;
        }

        sourceName = argv[1];

        std::ostringstream buffer;
        buffer << inputFile.rdbuf();
        src = buffer.str();

        if (!src.empty() && src.back() != '\n') {
            src.push_back('\n');
        }
    } else {
        std::string line;
        while (std::getline(std::cin, line)) {
            src += line + "\n";
        }
    }

    Runtime runtime;
    runtime.source = src;
    runtime.sourceName = sourceName;

    Lexer lexer(src, sourceName);
    Parser parser(lexer.tokenize(), src, sourceName);
    Lexer lexer(src);
    Parser parser(lexer.tokenize(), src);
    auto program = parser.parseProgram();

    runtime.execute(program);
    runtime.flush();

    return 0;
}
