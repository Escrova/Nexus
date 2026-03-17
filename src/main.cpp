#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"


namespace {
bool hasNxtExtension(const std::string &path) {
    static constexpr const char *kExt = ".nxt";
    const std::size_t extLen = 4;
    if (path.size() < extLen) {
        return false;
    }
    return path.compare(path.size() - extLen, extLen, kExt) == 0;
}
}  // namespace

int main(int argc, char **argv) {
    std::string src;
    std::string sourceName = "<stdin>";

    if (argc > 1) {
        const std::string filePath = argv[1];
        if (!hasNxtExtension(filePath)) {
            std::cerr << "Expected a .nxt source file, got: " << filePath << std::endl;
            return 1;
        }

        std::ifstream inputFile(filePath);
        if (!inputFile) {
            std::cerr << "Failed to open source file: " << argv[1] << std::endl;
            return 1;
        }

        sourceName = filePath;

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
    auto program = parser.parseProgram();

    runtime.execute(program);
    runtime.flush();

    return 0;
}

