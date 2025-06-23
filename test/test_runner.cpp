#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../src/analysis/semantic.hpp"
#include "../src/lexer/lexer.hpp"
#include "../src/parser/parser.hpp"

bool runTest(const std::string &path, bool expectSuccess) {
  std::ifstream in(path);
  if (!in.is_open()) {
    std::cerr << "Failed to open test file: " << path << "\n";
    return false;
  }

  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string source = buffer.str();

  try {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();

    SemanticAnalyser analyser;
    analyser.analyse(program.get());

    if (!expectSuccess) {
      std::cerr << "[FAIL] Expected failure: " << path << "\n";
      return false;
    }
  } catch (...) {
    if (expectSuccess) {
      std::cerr << "[FAIL] Unexpected error in: " << path << "\n";
      return false;
    }
    return true;
  }

  std::cout << "[PASS] " << path << "\n";
  return true;
}

int main() {
  int passed = 0;
  int failed = 0;

  for (const auto &entry :
       std::filesystem::recursive_directory_iterator("tests")) {
    if (entry.is_regular_file()) {
      const std::string path = entry.path();
      if (path.ends_with(".qt")) {
        bool expectSuccess = !path.ends_with(".fail.qt");
        if (runTest(path, expectSuccess))
          passed++;
        else
          failed++;
      }
    }
  }

  std::cout << "\nTests Passed: " << passed << ", Failed: " << failed << "\n";
  return failed > 0 ? 1 : 0;
}
