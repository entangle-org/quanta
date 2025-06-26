// visitor_base.hpp
#pragma once

// Forward declarations for shared nodes
struct Program;
struct FunctionDeclaration;
struct BlockStatement;
struct ReturnStatement;
struct ExpressionStatement;
struct AssignmentStatement;
struct BinaryExpression;
struct UnaryExpression;
struct LiteralExpression;
struct VariableExpression;
struct CallExpression;

struct BaseCodegenVisitor {
  virtual ~BaseCodegenVisitor() = default;

  virtual void visit(Program &) = 0;
  virtual void visit(FunctionDeclaration &) = 0;
  virtual void visit(BlockStatement &) = 0;
  virtual void visit(ReturnStatement &) = 0;
  virtual void visit(ExpressionStatement &) = 0;
  virtual void visit(AssignmentStatement &) = 0;
  virtual void visit(BinaryExpression &) = 0;
  virtual void visit(UnaryExpression &) = 0;
  virtual void visit(LiteralExpression &) = 0;
  virtual void visit(VariableExpression &) = 0;
  virtual void visit(CallExpression &) = 0;
};
