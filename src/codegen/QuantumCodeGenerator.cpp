#include "QuantumCodeGenerator.hpp"
#include <iostream>
#include <memory>
#include <sstream>

std::string QuantumCodeGenerator::generate(const std::shared_ptr<Program>& program) {
    output.str("");
    output << "OPENQASM 3.0;\n";
    output << "qubit[1] q;\n"; // simple placeholder, ideally we'd infer and declare properly

    for (const auto& stmt : program->statements) {
        generateStmt(stmt);
    }

    return output.str();
}

void QuantumCodeGenerator::generateStmt(const std::shared_ptr<Statement>& stmt) {
    if (auto var = std::dynamic_pointer_cast<VariableDecl>(stmt)) {
        if (var->primitive == "qubit") {
            output << "// Declare qubit " << var->name << "\n";
            output << "qubit " << var->name << ";\n";
            // Handle initialiser if present
            if (var->initialiser) {
                output << "// Initialize " << var->name << " = ";
                generateExpr(var->initialiser);
                output << "\n";
            }
        }
    } else if (auto func = std::dynamic_pointer_cast<FunctionDecl>(stmt)) {
        if (!func->isQuantum)
            return;
        output << "// Quantum function: " << func->name << "\n";

        for (const auto& s : func->body) {
            generateStmt(s);
        }
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
        generateExpr(exprStmt->expression);
        output << ";\n";
    } else {
        output << "// Unknown quantum statement\n";
    }
}

void QuantumCodeGenerator::generateExpr(const std::shared_ptr<Expression>& expr) {
    if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
        output << lit->value;
    } else if (auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
        output << id->name;
    } else if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        output << call->callee << "(";
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            if (i > 0)
                output << ", ";
            generateExpr(call->arguments[i]);
        }
        output << ")";
    } else if (auto assign = std::dynamic_pointer_cast<AssignmentExpr>(expr)) {
        output << assign->name << " = ";
        generateExpr(assign->value);
    } else {
        output << "/* unknown quantum expr */";
    }
}