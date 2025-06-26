// driver.cpp
#include "driver.hpp"
#include "cppgen.hpp"
#include "oqasmgen.hpp"

#include <fstream>

void CodegenDriver::generate(const Program &program, const std::string &backend,
                             const std::string &outputPath) {
  std::ofstream out(outputPath);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open output file: " + outputPath);
  }

  if (backend == "cpp") {
    CppGenerator gen;
    program.accept(gen);
    out << gen.str();
  } else if (backend == "qasm") {
    QasmGenerator gen;
    program.accept(gen);
    out << gen.str();
  } else {
    throw std::runtime_error("Unsupported backend: " + backend);
  }
}
