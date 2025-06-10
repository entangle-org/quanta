#pragma once

#include "CodeGenerator.hpp"

class ClassicalCodeGenerator : public CodeGenerator {
public:
    std::string generate(const std::shared_ptr<Program>& program) override;

private:
    std::ostringstream output;

    void generateStmt(const std::shared_ptr<Statement>& stmt);
    void generateExpr(const std::shared_ptr<Expression>& expr);

    std::string ClassicalCodeGenerator::toCppType(const std::string& quantaType);
};