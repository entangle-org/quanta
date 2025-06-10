#pragma once

#include <memory>
#include <string>
#include <vector>

struct Statement;
struct Expression;

struct Program {
    std::vector<std::shared_ptr<Statement>> statements;
};

enum class StatementType { VariableDecl, FunctionDecl, ExpressionStmt, ReturnStmt };

struct Statement {
    StatementType type;
    virtual ~Statement() = default;
};

struct VariableDecl : public Statement {
    std::string primitive;
    std::string name;
    std::shared_ptr<Expression> initialiser;

    VariableDecl(const std::string& primitive, const std::string& name, std::shared_ptr<Expression> init)
        : primitive(primitive)
        , name(name)
        , initialiser(std::move(init)) {
        type = StatementType::VariableDecl;
    }
};

struct Parameter {
    std::string primitive;
    std::string name;
};

struct FunctionDecl : public Statement {
    bool isQuantum;
    std::string name;
    std::vector<Parameter> params;
    std::string returnType;
    std::vector<std::shared_ptr<Statement>> body;

    FunctionDecl(bool isQuantum, const std::string& name, std::vector<Parameter> params, const std::string& returnType)
        : isQuantum(isQuantum)
        , name(name)
        , params(std::move(params))
        , returnType(returnType) {
        type = StatementType::FunctionDecl;
    }
};

enum class ExpressionType { Literal, Identifier, Binary, Call, Assignment };

struct Expression {
    ExpressionType type;
    virtual ~Expression() = default;
};

struct ExpressionStmt : public Statement {
    std::shared_ptr<Expression> expression;

    explicit ExpressionStmt(std::shared_ptr<Expression> expression)
        : expression(std::move(expression)) {}
};

struct LiteralExpr : public Expression {
    std::string value;

    explicit LiteralExpr(std::string value)
        : value(std::move(value)) {
        type = ExpressionType::Literal;
    }
};

struct IdentifierExpr : public Expression {
    std::string name;

    explicit IdentifierExpr(std::string name)
        : name(std::move(name)) {
        type = ExpressionType::Identifier;
    }
};

struct BinaryExpr : public Expression {
    std::shared_ptr<Expression> left;
    std::string op;
    std::shared_ptr<Expression> right;

    BinaryExpr(std::shared_ptr<Expression> left, std::string op, std::shared_ptr<Expression> right)
        : left(std::move(left))
        , op(std::move(op))
        , right(std::move(right)) {
        type = ExpressionType::Binary;
    }
};

struct CallExpr : public Expression {
    std::string callee;
    std::vector<std::shared_ptr<Expression>> arguments;

    CallExpr(std::string callee, std::vector<std::shared_ptr<Expression>> args)
        : callee(std::move(callee))
        , arguments(std::move(args)) {
        type = ExpressionType::Call;
    }
};

struct AssignmentExpr : public Expression {
    std::string name;
    std::shared_ptr<Expression> value;

    AssignmentExpr(std::string name, std::shared_ptr<Expression> value)
        : name(std::move(name))
        , value(std::move(value)) {
        type = ExpressionType::Assignment;
    }
};