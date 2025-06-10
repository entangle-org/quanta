#pragma once

#include "../ast/ast.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum class Type { Int, Float, String, Char, Qubit, Void, Unknown };

struct Symbol {
    std::string name;
    Type type;
};

class SemanticAnalyser {
public:
    void analyse(const std::shared_ptr<Program>& program);

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;

    void enterScope();
    void exitScope();
    void declare(const std::string& name, Type type);
    Symbol resolve(const std::string& name);

    Type analyseStmt(const std::shared_ptr<Statement>& stmt);
    Type analyseExpr(const std::shared_ptr<Expression>& expr);

    Type toType(const std::string& typeStr);
};