#include "ast/ast.hpp"
#include "cppgen.hpp"

void CppGenerator::visit(Program &node) {
  out << "#include <iostream>\n#include <string>\n\n";
  for (auto &cls : node.classes)
    cls->accept(*this);
  for (auto &fn : node.functions)
    if (!fn->hasQuantumAnnotation)
      fn->accept(*this);
  out << "\nint main() { return main__(); }\n";
}

void CppGenerator::visit(FunctionDeclaration &node) {
  std::string ret = "void";
  if (auto *pt = dynamic_cast<PrimitiveType *>(node.returnType.get()))
    ret = pt->name;
  std::string name = (node.name == "main") ? "main__" : node.name;
  out << ret << " " << name << "(";
  for (size_t i = 0; i < node.params.size(); ++i) {
    node.params[i]->accept(*this);
    if (i + 1 < node.params.size())
      out << ", ";
  }
  out << ") {\n";
  if (node.body)
    node.body->accept(*this);
  out << "}\n\n";
}

void CppGenerator::visit(ClassDeclaration &node) {
  out << "class " << node.name << " {\npublic:\n";
  for (auto &var : node.members)
    var->accept(*this);
  for (auto &fn : node.methods)
    fn->accept(*this);
  out << "};\n\n";
}

void CppGenerator::visit(VariableDeclaration &node) {
  out << "  ";
  if (node.isFinal)
    out << "const ";
  if (auto *pt = dynamic_cast<PrimitiveType *>(node.varType.get()))
    out << pt->name << " ";
  out << node.name;
  if (node.initializer) {
    out << " = ";
    node.initializer->accept(*this);
  }
  out << ";\n";
}

void CppGenerator::visit(ReturnStatement &node) {
  out << "  return ";
  if (node.value)
    node.value->accept(*this);
  out << ";\n";
}

void CppGenerator::visit(BlockStatement &node) {
  for (auto &stmt : node.statements) {
    stmt->accept(*this);
  }
}

void CppGenerator::visit(ExpressionStatement &node) {
  out << "  ";
  if (node.expression)
    node.expression->accept(*this);
  out << ";\n";
}

void CppGenerator::visit(IfStatement &node) {
  out << "  if (";
  node.condition->accept(*this);
  out << ") {\n";
  node.thenBranch->accept(*this);
  out << "  }";
  if (node.elseBranch) {
    out << " else {\n";
    node.elseBranch->accept(*this);
    out << "  }";
  }
  out << "\n";
}

void CppGenerator::visit(ForStatement &node) {
  out << "  for (";
  if (node.initializer)
    node.initializer->accept(*this);
  out << " ";
  node.condition->accept(*this);
  out << "; ";
  node.increment->accept(*this);
  out << ") {\n";
  node.body->accept(*this);
  out << "  }\n";
}

void CppGenerator::visit(EchoStatement &node) {
  out << "  std::cout << ";
  node.value->accept(*this);
  out << " << std::endl;\n";
}

void CppGenerator::visit(AssignmentStatement &node) {
  out << "  " << node.name << " = ";
  node.value->accept(*this);
  out << ";\n";
}

void CppGenerator::visit(BinaryExpression &node) {
  out << "(";
  node.left->accept(*this);
  out << " " << node.op << " ";
  node.right->accept(*this);
  out << ")";
}

void CppGenerator::visit(UnaryExpression &node) {
  out << "(" << node.op;
  node.right->accept(*this);
  out << ")";
}

void CppGenerator::visit(LiteralExpression &node) { out << node.value; }

void CppGenerator::visit(VariableExpression &node) { out << node.name; }

void CppGenerator::visit(CallExpression &node) {
  if (auto *ve = dynamic_cast<VariableExpression *>(node.callee.get())) {
    if (ve->name == "echo") {
      out << "std::cout << ";
      for (size_t i = 0; i < node.arguments.size(); ++i) {
        node.arguments[i]->accept(*this);
        if (i + 1 < node.arguments.size())
          out << " << ";
      }
      out << " << std::endl";
      return;
    }
    out << ve->name << "(";
    for (size_t i = 0; i < node.arguments.size(); ++i) {
      node.arguments[i]->accept(*this);
      if (i + 1 < node.arguments.size())
        out << ", ";
    }
    out << ")";
  }
}

void CppGenerator::visit(AssignmentExpression &node) {
  out << node.name << " = ";
  node.value->accept(*this);
}

void CppGenerator::visit(ConstructorCallExpression &node) {
  out << node.className << "(";
  for (size_t i = 0; i < node.arguments.size(); ++i) {
    node.arguments[i]->accept(*this);
    if (i + 1 < node.arguments.size())
      out << ", ";
  }
  out << ")";
}

void CppGenerator::visit(MemberAccessExpression &node) {
  node.object->accept(*this);
  out << "." << node.member;
}

void CppGenerator::visit(IndexExpression &node) {
  node.collection->accept(*this);
  out << "[";
  node.index->accept(*this);
  out << "]";
}

void CppGenerator::visit(ParenthesizedExpression &node) {
  out << "(";
  node.expression->accept(*this);
  out << ")";
}

void CppGenerator::visit(PrimitiveType &) {}
void CppGenerator::visit(VoidType &) {}
void CppGenerator::visit(ArrayType &) {}
void CppGenerator::visit(ObjectType &) {}
void CppGenerator::visit(LogicalType &) {}

void CppGenerator::visit(Parameter &node) {
  if (auto *pt = dynamic_cast<PrimitiveType *>(node.type.get()))
    out << pt->name << " " << node.name;
}

void CppGenerator::visit(AnnotationNode &) {}
