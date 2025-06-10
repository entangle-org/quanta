#include "SemanticAnalysis/SemanticAnalyser.hpp"
#include "ast/ASTPrinter.hpp"
#include "lexer/tokenizer.hpp"
#include "parser/parser.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: quanta <filename.qnta>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Tokenizer tokenizer(source);
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(tokens);
    auto program = parser.parse();

    SemanticAnalyser analyser;
    analyser.analyse(program);

    ASTPrinter::print(program);
    return 0;
}