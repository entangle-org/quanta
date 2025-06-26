#pragma once

#include "ast/ast.hpp"
#include "cppgen.hpp"
#include "oqasmgen.hpp"
#include <string>

class CodegenDriver {
public:
  static void generate(const Program &program, const std::string &backend,
                       const std::string &outputFile);
};
