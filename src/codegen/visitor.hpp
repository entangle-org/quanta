#pragma once

// Forward declarations of all AST node types
struct Program;
struct FunctionDeclaration;
struct ClassDeclaration;
struct VariableDeclaration;
struct ReturnStatement;
struct BlockStatement;
struct ExpressionStatement;
struct IfStatement;
struct ForStatement;
struct EchoStatement;
struct ResetStatement;
struct MeasureStatement;
struct AssignmentStatement;

// Expressions
struct BinaryExpression;
struct UnaryExpression;
struct LiteralExpression;
struct VariableExpression;
struct CallExpression;
struct IndexExpression;
struct ParenthesizedExpression;
struct MeasureExpression;
struct AssignmentExpression;
struct ConstructorCallExpression;
struct MemberAccessExpression;

struct CodegenVisitor {
  virtual ~CodegenVisitor() = default;

  // Top-level
  virtual void visit(Program &node) = 0;
  virtual void visit(FunctionDeclaration &node) = 0;
  virtual void visit(ClassDeclaration &node) = 0;

  // Statements
  virtual void visit(VariableDeclaration &node) = 0;
  virtual void visit(ReturnStatement &node) = 0;
  virtual void visit(BlockStatement &node) = 0;
  virtual void visit(ExpressionStatement &node) = 0;
  virtual void visit(IfStatement &node) = 0;
  virtual void visit(ForStatement &node) = 0;
  virtual void visit(EchoStatement &node) = 0;
  virtual void visit(ResetStatement &node) = 0;
  virtual void visit(MeasureStatement &node) = 0;
  virtual void visit(AssignmentStatement &node) = 0;

  // Expressions
  virtual void visit(BinaryExpression &node) = 0;
  virtual void visit(UnaryExpression &node) = 0;
  virtual void visit(LiteralExpression &node) = 0;
  virtual void visit(VariableExpression &node) = 0;
  virtual void visit(CallExpression &node) = 0;
  virtual void visit(IndexExpression &node) = 0;
  virtual void visit(ParenthesizedExpression &node) = 0;
  virtual void visit(MeasureExpression &node) = 0;
  virtual void visit(AssignmentExpression &node) = 0;
  virtual void visit(ConstructorCallExpression &node) = 0;
  virtual void visit(MemberAccessExpression &node) = 0;
};
