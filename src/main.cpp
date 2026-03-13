#include <fstream>
#include <iostream>
#include <sstream>
#include <iostream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"

int main(int argc, char **argv) {
    std::string src;

    if (argc > 1) {
        std::ifstream inputFile(argv[1]);
        if (!inputFile) {
            std::cerr << "Failed to open source file: " << argv[1] << std::endl;
            return 1;
        }

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
