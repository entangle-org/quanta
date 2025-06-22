#include "lexer/lexer.hpp"
#include <iostream>

int main() {
  std::string source = R"(
    @quantum
    function bell(qubit q0, qubit q1) -> void {
      h(q0);
      cx(q0, q1);
    }
  )";

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  for (const auto &token : tokens) {
    std::cout << "Token: " << static_cast<int>(token.type)
              << ", Value: " << token.value << ", Line: " << token.line
              << ", Column: " << token.column << std::endl;
  }

  return 0;
}