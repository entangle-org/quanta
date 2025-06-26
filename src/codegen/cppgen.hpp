#pragma once
#include "visitor_base.hpp"
#include <sstream>

// Forward declarations for C++-only nodes
struct VariableDeclaration;
struct IfStatement;
struct ForStatement;
struct EchoStatement;
struct ClassDeclaration;
struct ConstructorCallExpression;
struct MemberAccessExpression;
struct IndexExpression;
struct ParenthesizedExpression;
struct PrimitiveType;
struct ObjectType;
struct VoidType;
struct ArrayType;
struct AnnotationNode;
struct Parameter;

class CppGenerator : public BaseCodegenVisitor {
public:
  std::string str() const;

  // Common visitor implementations
  void visit(Program &) override;
  void visit(FunctionDeclaration &) override;
  void visit(BlockStatement &) override;
  void visit(ReturnStatement &) override;
  void visit(ExpressionStatement &) override;
  void visit(AssignmentStatement &) override;
  void visit(BinaryExpression &) override;
  void visit(UnaryExpression &) override;
  void visit(LiteralExpression &) override;
  void visit(VariableExpression &) override;
  void visit(CallExpression &) override;

  // C++-specific extensions
  void visit(VariableDeclaration &);
  void visit(IfStatement &);
  void visit(ForStatement &);
  void visit(EchoStatement &);
  void visit(ClassDeclaration &);
  void visit(ConstructorCallExpression &);
  void visit(MemberAccessExpression &);
  void visit(IndexExpression &);
  void visit(ParenthesizedExpression &);
  void visit(PrimitiveType &);
  void visit(ObjectType &);
  void visit(VoidType &);
  void visit(ArrayType &);
  void visit(AnnotationNode &);
  void visit(Parameter &);

private:
  std::ostringstream output;
};
