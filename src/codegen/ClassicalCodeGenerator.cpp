#include "ClassicalCodeGenerator.hpp"

std::string ClassicalCodeGenerator::generate(const std::shared_ptr<Program>& program) {
    output.str("");
    for (const auto& stmt : program->statements) {
        generateStmt(stmt);
    }
    return output.str();
}

void ClassicalCodeGenerator::generateStmt(const std::shared_ptr<Statement>& stmt) {
    if (auto var = std::dynamic_pointer_cast<VariableDecl>(stmt)) {
        output << toCppType(var->primitive) << " " << var->name;
        if (var->initialiser) {
            output << " = ";
            generateExpr(var->initialiser);
        }
        output << ";\n";
    } else if (auto func = std::dynamic_pointer_cast<FunctionDecl>(stmt)) {
        if (func->isQuantum)
            return;
        output << toCppType(func->returnType) << " " << func->name << "(";
        for (size_t i = 0; i < func->params.size(); ++i) {
            if (i > 0)
                output << ", ";
            output << toCppType(func->params[i].primitive) << " " << func->params[i].name;
        }
        output << ") {\n";
        for (const auto& s : func->body) {
            output << "  ";
            generateStmt(s);
        }
        output << "}\n";
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
        generateExpr(exprStmt->expression);
        output << ";\n";
    } else {
        output << "// Unknown statement\n";
    }
}

void ClassicalCodeGenerator::generateExpr(const std::shared_ptr<Expression>& expr) {
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
        output << "/* unknown expr */";
    }
}

std::string ClassicalCodeGenerator::toCppType(const std::string& quantaType) {
    if (quantaType == "int")
        return "int";
    if (quantaType == "float")
        return "float";
    if (quantaType == "char")
        return "char";
    if (quantaType == "string")
        return "std::string";
    if (quantaType == "void")
        return "void";
    return "auto";