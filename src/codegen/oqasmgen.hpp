#pragma once
#include "visitor_base.hpp"
#include <sstream>

// Forward declarations for QASM-only nodes
struct MeasureStatement;
struct ResetStatement;
struct MeasureExpression;
struct AnnotationNode;
struct Parameter;
struct PrimitiveType;
struct ArrayType;
struct VoidType;
struct LogicalType;

class QasmGenerator : public BaseCodegenVisitor {
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

  // QASM-specific extensions
  void visit(MeasureStatement &);
  void visit(ResetStatement &);
  void visit(MeasureExpression &);
  void visit(AnnotationNode &);
  void visit(Parameter &);
  void visit(PrimitiveType &);
  void visit(ArrayType &);
  void visit(VoidType &);
  void visit(LogicalType &);

private:
  std::ostringstream output;
};
