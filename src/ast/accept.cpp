#include "ast.hpp"
#include "codegen/visitor.hpp"

void AssignmentExpression::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void AssignmentStatement::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void BinaryExpression::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void BlockStatement::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void CallExpression::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void ClassDeclaration::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void ConstructorCallExpression::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void EchoStatement::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void ExpressionStatement::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void ForStatement::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void FunctionDeclaration::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void IfStatement::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void IndexExpression::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void LiteralExpression::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void MeasureExpression::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void MeasureStatement::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void MemberAccessExpression::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void ParenthesizedExpression::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void Program::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void ResetStatement::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void ReturnStatement::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void UnaryExpression::accept(CodegenVisitor &visitor) { visitor.visit(*this); }

void VariableDeclaration::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}

void VariableExpression::accept(CodegenVisitor &visitor) {
  visitor.visit(*this);
}
