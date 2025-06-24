#include "semantic.hpp"
#include <iostream>
#include <lexer/token.hpp>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

Scope::Scope(std::shared_ptr<Scope> parent) : parent(std::move(parent)) {}

void Scope::define(const std::string &name, const Symbol &symbol) {
  if (symbols.count(name)) {
    reportError("Duplicate symbol: " + name);
  }
  symbols[name] = symbol;
}

Symbol *Scope::resolve(const std::string &name) {
  if (symbols.count(name)) {
    return &symbols[name];
  }
  if (parent) {
    return parent->resolve(name);
  }
  return nullptr;
}

std::shared_ptr<Scope> Scope::getParent() { return parent; }

void Scope::reportError(const std::string &msg) {
  std::stringstream err;
  err << "[Quanta Semantic Error]\n" << msg << "\n";
  throw std::runtime_error(err.str());
}

void SemanticAnalyser::analyse(const Program *program) {
  currentScope = std::make_shared<Scope>();
  analyseProgram(program);
}

void SemanticAnalyser::enterScope() {
  currentScope = std::make_shared<Scope>(currentScope);
}

void SemanticAnalyser::exitScope() { currentScope = currentScope->getParent(); }

void SemanticAnalyser::declare(const std::string &name, const Symbol &symbol) {
  currentScope->define(name, symbol);
}

Symbol *SemanticAnalyser::lookup(const std::string &name) {
  return currentScope->resolve(name);
}

void SemanticAnalyser::analyseProgram(const Program *program) {
  for (const auto &cls : program->classes) {
    analyseClass(cls.get());
  }
  for (const auto &func : program->functions) {
    analyseFunction(func.get());
  }
  for (const auto &stmt : program->statements) {
    analyseStatement(stmt.get());
  }
}

void SemanticAnalyser::analyseClass(const ClassDeclaration *clazz) {
  for (const auto &member : clazz->members) {
    analyseVariableDeclaration(member.get());
  }
  for (const auto &method : clazz->methods) {
    analyseFunction(method.get());
  }
}

void SemanticAnalyser::analyseFunction(const FunctionDeclaration *func) {
  Symbol sym{func->name, func->returnType.get(), SymbolKind::Function, false};
  declare(func->name, sym);

  bool hasQuantum = false;
  bool hasAdjoint = false;
  for (const auto &ann : func->annotations) {
    if (ann->name == "quantum")
      hasQuantum = true;
    if (ann->name == "adjoint")
      hasAdjoint = true;
  }

  if (hasAdjoint && !hasQuantum) {
    reportError("@adjoint annotation is only allowed on @quantum functions");
  }

  if (hasQuantum) {
    Type *retType = func->returnType.get();
    if (!isVoidType(retType) && !isBitType(retType)) {
      reportError("@quantum functions must return void or bit");
    }
    inQuantumFunction = true;
  }

  enterScope();
  for (const auto &param : func->params) {
    Symbol paramSym{param->name, param->type.get(), SymbolKind::Parameter,
                    false};
    declare(param->name, paramSym);
  }

  analyseBlock(func->body.get());
  exitScope();

  if (!isVoidType(func->returnType.get())) {
    bool hasReturn = false;
    for (const auto &stmt : func->body->statements) {
      if (dynamic_cast<const ReturnStatement *>(stmt.get())) {
        hasReturn = true;
        break;
      }
    }
    if (!hasReturn) {
      reportError("Non-void function '" + func->name +
                  "' must have a return statement");
    }
  }

  inQuantumFunction = false;
}

// Statements

void SemanticAnalyser::analyseStatement(const Statement *stmt) {
  if (auto block = dynamic_cast<const BlockStatement *>(stmt)) {
    analyseBlock(block);
  } else if (auto var = dynamic_cast<const VariableDeclaration *>(stmt)) {
    analyseVariableDeclaration(var);
  } else if (auto ret = dynamic_cast<const ReturnStatement *>(stmt)) {
    analyseReturn(ret);
  } else if (auto iff = dynamic_cast<const IfStatement *>(stmt)) {
    analyseIf(iff);
  } else if (auto loop = dynamic_cast<const ForStatement *>(stmt)) {
    analyseFor(loop);
  } else if (auto echo = dynamic_cast<const EchoStatement *>(stmt)) {
    analyseEcho(echo);
  } else if (auto reset = dynamic_cast<const ResetStatement *>(stmt)) {
    analyseReset(reset);
  } else if (auto meas = dynamic_cast<const MeasureStatement *>(stmt)) {
    analyseMeasure(meas);
  } else if (auto assign = dynamic_cast<const AssignmentStatement *>(stmt)) {
    analyseAssignment(assign);
  }
}

void SemanticAnalyser::analyseBlock(const BlockStatement *block) {
  enterScope();
  for (const auto &stmt : block->statements) {
    analyseStatement(stmt.get());
  }
  exitScope();
}

void SemanticAnalyser::analyseVariableDeclaration(
    const VariableDeclaration *decl) {
  // Check @state annotation
  for (const auto &ann : decl->annotations) {
    if (ann->name == "state") {
      auto *type = decl->varType.get();
      auto *pt = dynamic_cast<PrimitiveType *>(type);
      if (!pt || pt->name != "qubit") {
        reportError("@state can only be used on qubit declarations");
      }

      static const std::unordered_set<std::string> validStates = {
          "0", "1", "+", "-", "i", "-i"};

      if (validStates.find(ann->value) == validStates.end()) {
        reportError("Invalid @state value: " + ann->value);
      }
    }
  }

  if (inQuantumFunction) {
    auto *type = decl->varType.get();
    auto *pt = dynamic_cast<PrimitiveType *>(type);
    if (!pt || (pt->name != "qubit" && pt->name != "bit")) {
      reportError(
          "Cannot declare classical variable inside a @quantum function");
    }
  }

  Symbol sym{decl->name, decl->varType.get(), SymbolKind::Variable,
             decl->isFinal};
  declare(decl->name, sym);

  if (decl->initializer) {
    analyseExpression(decl->initializer.get());
  }
}

void SemanticAnalyser::analyseReturn(const ReturnStatement *stmt) {
  if (stmt->value)
    analyseExpression(stmt->value.get());
}

void SemanticAnalyser::analyseIf(const IfStatement *stmt) {
  analyseExpression(stmt->condition.get());
  analyseStatement(stmt->thenBranch.get());
  if (stmt->elseBranch)
    analyseStatement(stmt->elseBranch.get());
}

void SemanticAnalyser::analyseFor(const ForStatement *stmt) {
  enterScope();
  if (stmt->initializer)
    analyseStatement(stmt->initializer.get());
  if (stmt->condition)
    analyseExpression(stmt->condition.get());
  if (stmt->increment)
    analyseExpression(stmt->increment.get());
  analyseStatement(stmt->body.get());
  exitScope();
}

void SemanticAnalyser::analyseEcho(const EchoStatement *stmt) {
  analyseExpression(stmt->value.get());
}

void SemanticAnalyser::analyseReset(const ResetStatement *stmt) {
  analyseExpression(stmt->target.get());
}

void SemanticAnalyser::analyseMeasure(const MeasureStatement *stmt) {
  analyseExpression(stmt->qubit.get());
}

void SemanticAnalyser::analyseAssignment(const AssignmentStatement *stmt) {
  auto sym = lookup(stmt->name);
  if (!sym) {
    reportError("Undeclared variable: " + stmt->name);
  }
  if (sym->isFinal) {
    reportError("Cannot assign to final variable: " + stmt->name);
  }
  if (sym->kind == SymbolKind::Function) {
    reportError("Cannot assign to function: " + stmt->name);
  }
  analyseExpression(stmt->value.get());
}

// Expressions

Type *SemanticAnalyser::analyseExpression(const Expression *expr) {
  if (auto bin = dynamic_cast<const BinaryExpression *>(expr)) {
    return analyseBinary(bin);
  } else if (auto unary = dynamic_cast<const UnaryExpression *>(expr)) {
    return analyseUnary(unary);
  } else if (auto lit = dynamic_cast<const LiteralExpression *>(expr)) {
    return analyseLiteral(lit);
  } else if (auto var = dynamic_cast<const VariableExpression *>(expr)) {
    return analyseVariable(var);
  } else if (auto call = dynamic_cast<const CallExpression *>(expr)) {
    return analyseCall(call);
  } else if (auto meas = dynamic_cast<const MeasureExpression *>(expr)) {
    return analyseMeasure(meas);
  } else if (auto assign = dynamic_cast<const AssignmentExpression *>(expr)) {
    return analyseAssignmentExpr(assign);
  } else if (auto paren = dynamic_cast<const ParenthesizedExpression *>(expr)) {
    return analyseExpression(paren->expression.get());
  }
  reportError("Unknown expression type");
}

Type *SemanticAnalyser::analyseBinary(const BinaryExpression *expr) {
  auto left = analyseExpression(expr->left.get());
  auto right = analyseExpression(expr->right.get());

  if (!isSameType(left, right)) {
    std::stringstream err;
    err << "[Type Error] Operands must be of the same type (got "
        << typeToString(left) << " and " << typeToString(right) << ")\n";
    reportError(err.str());
  }

  if (expr->op == "+" || expr->op == "-" || expr->op == "*" ||
      expr->op == "/" || expr->op == "%") {
    if (!isNumeric(left)) {
      std::stringstream err;
      err << "[Type Error] Operator '" << expr->op
          << "' requires numeric types (got " << typeToString(left) << ")\n";
      reportError(err.str());
    }
    return left;
  }

  // Comparisons
  if (expr->op == ">" || expr->op == "<" || expr->op == ">=" ||
      expr->op == "<=") {
    return new PrimitiveType("bit");
  }

  std::stringstream err;
  err << "[Semantic Error] Unsupported binary operator: " << expr->op << "\n";
  reportError(err.str());
}

Type *SemanticAnalyser::analyseUnary(const UnaryExpression *expr) {
  auto right = analyseExpression(expr->right.get());

  if (expr->op == "-") {
    if (!isNumeric(right)) {
      std::stringstream err;
      err << "[Type Error] Unary '-' requires numeric type (got "
          << typeToString(right) << ")\n";
      reportError(err.str());
    }
    return right;
  }

  std::stringstream err;
  err << "[Semantic Error] Unsupported unary operator: " << expr->op << "\n";
  reportError(err.str());
}

Type *SemanticAnalyser::analyseLiteral(const LiteralExpression *expr) {
  const std::string &val = expr->value;

  if (val.find('.') != std::string::npos && val.back() == 'f') {
    return new PrimitiveType("float");
  }
  if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
    return new PrimitiveType("string");
  }
  if (val.size() == 3 && val.front() == '\'' && val.back() == '\'') {
    return new PrimitiveType("char");
  }
  return new PrimitiveType("int");
}

Type *SemanticAnalyser::analyseVariable(const VariableExpression *expr) {
  auto sym = lookup(expr->name);
  if (!sym) {
    std::stringstream err;
    err << "[Semantic Error] Undeclared variable: " << expr->name << "\n";
    reportError(err.str());
  }
  return sym->type;
}

Type *SemanticAnalyser::analyseCall(const CallExpression *expr) {
  auto sym = lookup(expr->callee);
  if (!sym || sym->kind != SymbolKind::Function) {
    std::stringstream err;
    err << "[Semantic Error] '" << expr->callee << "' is not a function\n";
    reportError(err.str());
  }

  const auto &funcType = sym->type;
  // TODO: match arguments vs func declaration from AST (symbol table doesn't
  // yet store parameters)

  // For now, just check all args are valid expressions
  for (const auto &arg : expr->arguments) {
    analyseExpression(arg.get());
  }

  return funcType;
}

Type *SemanticAnalyser::analyseMeasure(const MeasureExpression *expr) {
  auto t = analyseExpression(expr->qubit.get());
  if (auto p = dynamic_cast<PrimitiveType *>(t)) {
    if (p->name != "qubit") {
      std::stringstream err;
      err << "[Semantic Error] measure expects a qubit\n";
      reportError(err.str());
    }
    return new PrimitiveType("bit");
  }
  std::stringstream err;
  err << "[Semantic Error] measure expects a qubit\n";
  reportError(err.str());
}

Type *
SemanticAnalyser::analyseAssignmentExpr(const AssignmentExpression *expr) {
  auto sym = lookup(expr->name);
  if (!sym) {
    std::stringstream err;
    err << "[Semantic Error] Undeclared variable: " << expr->name << "\n";
    reportError(err.str());
  }
  if (sym->isFinal) {
    std::stringstream err;
    err << "[Semantic Error] Cannot assign to final variable: " << expr->name
        << "\n";
    reportError(err.str());
  }

  Type *rhs = analyseExpression(expr->value.get());
  if (!isSameType(sym->type, rhs)) {
    std::stringstream err;
    err << "[Type Error] Cannot assign value of type " << typeToString(rhs)
        << " to variable of type " << typeToString(sym->type) << "\n";
    reportError(err.str());
  }

  return sym->type;
}

// Type helpers

Type *SemanticAnalyser::evaluateType(const std::unique_ptr<Type> &t) {
  return t.get();
}

bool SemanticAnalyser::isSameType(Type *a, Type *b) {
  auto *pa = dynamic_cast<PrimitiveType *>(a);
  auto *pb = dynamic_cast<PrimitiveType *>(b);
  if (pa && pb)
    return pa->name == pb->name;
  return false;
}

bool SemanticAnalyser::isNumeric(Type *t) {
  auto *pt = dynamic_cast<PrimitiveType *>(t);
  return pt && (pt->name == "int" || pt->name == "float");
}

bool SemanticAnalyser::isBitType(Type *t) {
  auto *pt = dynamic_cast<PrimitiveType *>(t);
  return pt && pt->name == "bit";
}

bool SemanticAnalyser::isVoidType(Type *t) {
  auto *pt = dynamic_cast<PrimitiveType *>(t);
  return pt && pt->name == "void";
}

std::string SemanticAnalyser::typeToString(Type *t) {
  if (auto *pt = dynamic_cast<PrimitiveType *>(t))
    return pt->name;
  if (auto *lt = dynamic_cast<LogicalType *>(t))
    return "logical<" + lt->code + ">";
  if (dynamic_cast<VoidType *>(t))
    return "void";
  return "unknown";
}

// Error handling
void SemanticAnalyser::reportError(const std::string &msg) {
  std::stringstream err;
  err << "[Semantic Error] " << msg;
  throw std::runtime_error(err.str());
}

void SemanticAnalyser::reportError(const std::string &msg, const Token &token) {
  std::stringstream err;
  err << "[Semantic Error] Line " << token.line << ", Col " << token.column
      << ", Token = '" << token.value << "': " << msg;
  throw std::runtime_error(err.str());
}

void SemanticAnalyser::reportError(const std::string &msg,
                                   const ASTNode *node) {
  std::stringstream err;
  err << "[Semantic Error] " << msg;
  throw std::runtime_error(err.str());
}