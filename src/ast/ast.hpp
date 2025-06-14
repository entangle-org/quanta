#pragma once

#include <memory>
#include <string>
#include <vector>

struct Statement;
struct Expression;

struct Program {
    std::vector<std::shared_ptr<Statement>> statements;
};

enum class StatementType { VariableDecl, FunctionDecl, ExpressionStmt, ReturnStmt, ResetStmt, AdjointStmt };

struct Statement {
    StatementType type;
    virtual ~Statement() = default;
};

struct VariableDecl : public Statement {
    std::string primitive;
    std::string name;
    std::shared_ptr<Expression> initialiser;
    bool isFinal = false;
    bool isArray = false;
    int arraySize = 0;

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
    bool isQuantum = false;
    bool isAdjoint = false;
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

struct ResetStmt : public Statement {
    std::shared_ptr<Expression> target;
    ResetStmt(std::shared_ptr<Expression> t)
        : target(std::move(t)) {
        type = StatementType::ResetStmt;
    }
};

struct AdjointStmt : public Statement {
    std::string callee;
    std::vector<std::shared_ptr<Expression>> args;
    AdjointStmt(std::string c, std::vector<std::shared_ptr<Expression>> a)
        : callee(std::move(c))
        , args(std::move(a)) {
        type = StatementType::AdjointStmt;
    }
};

struct ReturnStmt : public Statement {
    std::shared_ptr<Expression> value;
    explicit ReturnStmt(std::shared_ptr<Expression> val)
        : value(std::move(val)) {
        type = StatementType::ReturnStmt;
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

struct Annotation {
    std::string name;
    std::string argument;

    Annotation(const std::string& name, const std::string& arg = "")
        : name(name)
        , argument(arg) {}
};