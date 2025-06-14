#pragma once

#include "ast.hpp"
#include <iostream>
#include <memory>

class ASTPrinter {
public:
    static void print(const std::shared_ptr<Program>& program, int indent = 0);

private:
    static void printIndent(int indent);
    static void print(const std::shared_ptr<Statement>& stmt, int indent);
    static void print(const std::shared_ptr<Expression>& expr, int indent);
};
