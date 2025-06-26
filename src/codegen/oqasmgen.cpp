#include "oqasmgen.hpp"
#include "ast/ast.hpp"

void QasmGenerator::visit(Program &node) {
  out << "OPENQASM 3.0;\n";
  out << "include \"stdgates.inc\";\n";

  // Count total qubits
  for (const auto &stmt : node.statements) {
    if (auto *decl = dynamic_cast<VariableDeclaration *>(stmt.get())) {
      if (auto *type = dynamic_cast<PrimitiveType *>(decl->varType.get())) {
        if (type->name == "qubit") {
          qubitCount++;
        }
      }
    }
  }

  if (qubitCount > 0) {
    out << "qubit[" << qubitCount << "] q;\n";
  }

  for (const auto &func : node.functions) {
    if (func->hasQuantumAnnotation) {
      func->accept(*this);
    }
  }
}

void QasmGenerator::visit(FunctionDeclaration &node) {
  out << "\ndef " << node.name << "(";
  for (size_t i = 0; i < node.params.size(); ++i) {
    auto &param = node.params[i];
    if (auto *pt = dynamic_cast<PrimitiveType *>(param->type.get())) {
      if (pt->name == "qubit") {
        out << "qubit " << param->name;
        if (i + 1 < node.params.size())
          out << ", ";
      }
    }
  }
  out << ") {\n";
  if (node.body)
    node.body->accept(*this);
  out << "}\n";
}

void QasmGenerator::visit(ReturnStatement &node) {
  out << "  // return ";
  if (node.value) {
    node.value->accept(*this);
  }
  out << "\n";
}

void QasmGenerator::visit(BlockStatement &node) {
  for (const auto &stmt : node.statements) {
    stmt->accept(*this);
  }
}

void QasmGenerator::visit(ExpressionStatement &node) {
  if (node.expression) {
    out << "  ";
    node.expression->accept(*this);
    out << ";\n";
  }
}

void QasmGenerator::visit(MeasureStatement &node) {
  if (auto *ve = dynamic_cast<VariableExpression *>(node.qubit.get())) {
    out << "  bit result = measure " << ve->name << ";\n";
  }
}

void QasmGenerator::visit(ResetStatement &node) {
  if (auto *ve = dynamic_cast<VariableExpression *>(node.target.get())) {
    out << "  reset " << ve->name << ";\n";
  }
}

void QasmGenerator::visit(AssignmentStatement &node) {
  out << "  " << node.name << " = ";
  node.value->accept(*this);
  out << ";\n";
}

void QasmGenerator::visit(BinaryExpression &node) {
  out << "(";
  node.left->accept(*this);
  out << " " << node.op << " ";
  node.right->accept(*this);
  out << ")";
}

void QasmGenerator::visit(UnaryExpression &node) {
  out << "(" << node.op;
  node.right->accept(*this);
  out << ")";
}

void QasmGenerator::visit(LiteralExpression &node) { out << node.value; }

void QasmGenerator::visit(VariableExpression &node) { out << node.name; }

void QasmGenerator::visit(CallExpression &node) {
  if (auto *callee = dynamic_cast<VariableExpression *>(node.callee.get())) {
    out << "  " << callee->name << "(";
    for (size_t i = 0; i < node.arguments.size(); ++i) {
      node.arguments[i]->accept(*this);
      if (i + 1 < node.arguments.size())
        out << ", ";
    }
    out << ");\n";
  }
}

void QasmGenerator::visit(MeasureExpression &node) {
  out << "measure ";
  node.qubit->accept(*this);
}

void QasmGenerator::visit(AssignmentExpression &node) {
  out << node.name << " = ";
  node.value->accept(*this);
}
