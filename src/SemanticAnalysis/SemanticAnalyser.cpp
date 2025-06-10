#include "SemanticAnalyser.hpp"
#include <iostream>

void SemanticAnalyser::analyse(const std::shared_ptr<Program>& program) {
    enterScope();
    for (const auto& stmt : program->statements) {
        analyseStmt(stmt);
    }
    exitScope();
}

void SemanticAnalyser::enterScope() {
    scopes.emplace_back();
}

void SemanticAnalyser::exitScope() {
    scopes.pop_back();
}

void SemanticAnalyser::declare(const std::string& name, Type type) {
    if (scopes.back().count(name)) {
        std::cerr << "[Semantic Error] Redeclaration of '" << name << "'\n";
        std::exit(1);
    }
    scopes.back()[name] = Symbol{name, type};
}

Symbol SemanticAnalyser::resolve(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->count(name)) {
            return (*it)[name];
        }
    }
    std::cerr << "[Semantic Error] Undeclared identifier '" << name << "'\n";
    std::exit(1);
}

Type SemanticAnalyser::analyseStmt(const std::shared_ptr<Statement>& stmt) {
    if (auto var = std::dynamic_pointer_cast<VariableDecl>(stmt)) {
        Type varType = toType(var->primitive);
        if (var->initialiser) {
            Type initType = analyseExpr(var->initialiser);
            if (initType != varType && initType != Type::Unknown) {
                std::cerr << "[Semantic Error] Type mismatch for variable '" << var->name << "': expected " << var->primitive << "\n";
                std::exit(1);
            }
        }
        declare(var->name, varType);
        return varType;
    }

    if (auto func = std::dynamic_pointer_cast<FunctionDecl>(stmt)) {
        declare(func->name, toType(func->returnType));
        enterScope();
        for (const auto& param : func->params) {
            declare(param.name, toType(param.primitive));
        }
        for (const auto& s : func->body) {
            analyseStmt(s);
        }
        exitScope();
        return Type::Void;
    }

    if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
        analyseExpr(exprStmt->expression);
        return Type::Void;
    }

    return Type::Unknown;
}

Type SemanticAnalyser::analyseExpr(const std::shared_ptr<Expression>& expr) {
    if (auto literal = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
        const std::string& val = literal->value;
        if (val.starts_with("\""))
            return Type::String;
        if (val.starts_with("'"))
            return Type::Char;
        if (val.find('.') != std::string::npos)
            return Type::Float;
        if (val.starts_with("@"))
            return Type::Qubit;
        return Type::Int;
    }

    if (auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
        return resolve(id->name).type;
    }

    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        resolve(call->callee);
        for (const auto& arg : call->arguments) {
            analyseExpr(arg);
        }
        return Type::Unknown;
    }

    if (auto assign = std::dynamic_pointer_cast<AssignmentExpr>(expr)) {
        Symbol sym = resolve(assign->name);
        Type rhsType = analyseExpr(assign->value);
        if (sym.type != rhsType && rhsType != Type::Unknown) {
            std::cerr << "[Semantic Error] Type mismatch in assignment to '" << assign->name << "'\n";
            std::exit(1);
        }
        return sym.type;
    }

    return Type::Unknown;
}

Type SemanticAnalyser::toType(const std::string& typeStr) {
    if (typeStr == "int")
        return Type::Int;
    if (typeStr == "float")
        return Type::Float;
    if (typeStr == "string")
        return Type::String;
    if (typeStr == "char")
        return Type::Char;
    if (typeStr == "qubit")
        return Type::Qubit;
    if (typeStr == "void")
        return Type::Void;
    return Type::Unknown;
}