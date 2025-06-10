#pragma once

#include "../ast/ast.hpp"
#include <memory>
#include <sstream>
#include <string>

class CodeGenerator {
public:
    virtual std::string generate(const std::shared_ptr<Program>& program);
    virtual ~CodeGenerator() = default;
};