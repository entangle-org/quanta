#include "ast_printer.hpp"
#include <iostream>

void ASTPrinter::print(const std::shared_ptr<Program>& program, int indent) {
    for (const auto& stmt : program->statements) {
        print(stmt, indent);
    }
}

void ASTPrinter::printIndent(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
}

void ASTPrinter::print(const std::shared_ptr<Statement>& stmt, int indent) {
    printIndent(indent);
    if (auto func = std::dynamic_pointer_cast<FunctionDecl>(stmt)) {
        std::cout << (func->isQuantum ? "Quantum" : "Classical") << " Function: " << func->name << " -> " << func->returnType << "\n";
        for (const auto& param : func->params) {
            printIndent(indent + 1);
            std::cout << "Param: " << param.primitive << " " << param.name << "\n";
        }
        for (const auto& s : func->body) {
            print(s, indent + 1);
        }
    } else if (auto var = std::dynamic_pointer_cast<VariableDecl>(stmt)) {
        std::cout << "Variable: " << var->primitive << " " << var->name << " = ";
        if (var->initialiser) {
            print(var->initialiser, 0);
        } else {
            std::cout << "(uninitialized)\n";
        }
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
        std::cout << "ExprStmt: ";
        print(exprStmt->expression, 0);
    } else {
        std::cout << "Unknown statement\n";
    }
}

void ASTPrinter::print(const std::shared_ptr<Expression>& expr, int indent) {
    printIndent(indent);
    if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
        std::cout << "Literal: " << lit->value << "\n";
    } else if (auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
        std::cout << "Identifier: " << id->name << "\n";
    } else if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::cout << "Call: " << call->callee << "(\n";
        for (const auto& arg : call->arguments) {
            printIndent(indent + 1);
            print(arg, indent + 1);
        }
        printIndent(indent);
        std::cout << ")\n";
    } else if (auto assign = std::dynamic_pointer_cast<AssignmentExpr>(expr)) {
        std::cout << "Assign: " << assign->name << " = ";
        print(assign->value, indent + 1);
        std::cout << "\n";
    } else {
        std::cout << "Unknown expression\n";
    }
}