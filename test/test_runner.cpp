#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/analysis/semantic.hpp"
#include "../src/lexer/lexer.hpp"
#include "../src/parser/parser.hpp"

namespace fs = std::filesystem;

std::string colorize(const std::string &text, const std::string &colorCode) {
  return "\033[" + colorCode + "m" + text + "\033[0m";
}

bool runTest(const std::string &path, bool expectSuccess) {
  std::ifstream in(path);
  if (!in.is_open()) {
    std::cerr << colorize("[ERROR] Cannot open test file: " + path, "1;31")
              << "\n";
    return false;
  }

  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string source = buffer.str();

  std::cout << colorize("[INFO] Running test: ", "1;34") << path
            << " (expectSuccess=" << expectSuccess << ")\n";

  try {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto program = parser.parse();

    SemanticAnalyser analyser;
    analyser.analyse(program.get());

    if (!expectSuccess) {
      std::cout << colorize("[FAIL] Expected failure but passed: ", "1;31")
                << path << "\n";
      return false;
    }
  } catch (const std::exception &e) {
    if (expectSuccess) {
      std::cout << colorize("[FAIL] Unexpected failure in: ", "1;31") << path
                << "\n";
      std::cerr << colorize(e.what(), "1;31") << "\n";
      return false;
    }
    std::cout << colorize("[PASS] ", "1;32") << path << "\n";
    return true;
  }

  std::cout << colorize("[PASS] ", "1;32") << path << "\n";
  return true;
}

int main() {
  const std::string testDir = "../test";
  const std::string validDir = testDir + "/valid";
  const std::string invalidDir = testDir + "/invalid";

  int total = 0, passed = 0;

  std::cout << colorize("\n[INFO] Running positive tests:\n\n", "1;34");
  for (const auto &entry : fs::directory_iterator(validDir)) {
    if (entry.is_regular_file()) {
      total++;
      if (runTest(entry.path().string(), true))
        passed++;
    }
  }

  std::cout << colorize("\n[INFO] Running negative tests:\n\n", "1;34");
  for (const auto &entry : fs::directory_iterator(invalidDir)) {
    if (entry.is_regular_file()) {
      total++;
      if (runTest(entry.path().string(), false))
        passed++;
    }
  }

  std::cout << "\n"
            << colorize("[SUMMARY] ", "1;36") << passed << "/" << total
            << " tests passed\n";
  return passed == total ? 0 : 1;
}
