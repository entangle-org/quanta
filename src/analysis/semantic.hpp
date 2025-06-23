#pragma once

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

enum class SymbolKind { Variable, Function, Class, Parameter };

struct Symbol {
  std::string name;
  Type *type;
  SymbolKind kind;
  bool isFinal;
};

class Scope {
public:
  Scope(std::shared_ptr<Scope> parent = nullptr);

  void define(const std::string &name, const Symbol &symbol);
  Symbol *resolve(const std::string &name);
  std::shared_ptr<Scope> getParent();

private:
  std::unordered_map<std::string, Symbol> symbols;
  std::shared_ptr<Scope> parent;
  void reportError(const std::string &msg);
};

class SemanticAnalyser {
public:
  void analyse(const Program *program);

private:
  std::shared_ptr<Scope> currentScope;
  bool inQuantumFunction = false;

  // Scope helpers
  void enterScope();
  void exitScope();
  void declare(const std::string &name, const Symbol &symbol);
  Symbol *lookup(const std::string &name);

  // Main visitors
  void analyseProgram(const Program *program);
  void analyseFunction(const FunctionDeclaration *func);
  void analyseClass(const ClassDeclaration *clazz);
  void analyseStatement(const Statement *stmt);
  Type *analyseExpression(const Expression *expr);

  // Statements
  void analyseVariableDeclaration(const VariableDeclaration *decl);
  void analyseBlock(const BlockStatement *block);
  void analyseReturn(const ReturnStatement *stmt);
  void analyseIf(const IfStatement *stmt);
  void analyseFor(const ForStatement *stmt);
  void analyseEcho(const EchoStatement *stmt);
  void analyseReset(const ResetStatement *stmt);
  void analyseMeasure(const MeasureStatement *stmt);
  void analyseAssignment(const AssignmentStatement *stmt);

  // Expressions
  Type *analyseBinary(const BinaryExpression *expr);
  Type *analyseUnary(const UnaryExpression *expr);
  Type *analyseLiteral(const LiteralExpression *expr);
  Type *analyseVariable(const VariableExpression *expr);
  Type *analyseCall(const CallExpression *expr);
  Type *analyseMeasure(const MeasureExpression *expr);
  Type *analyseAssignmentExpr(const AssignmentExpression *expr);

  // Type utils
  Type *evaluateType(const std::unique_ptr<Type> &t);
  bool isSameType(Type *a, Type *b);
  bool isNumeric(Type *t);
  bool isBitType(Type *t);
  bool isVoidType(Type *t);
  std::string typeToString(Type *t);

  // Error handling
  void reportError(const std::string &msg);
  void reportError(const std::string &msg, const Token &token);
  void reportError(const std::string &msg, const ASTNode *node);
};
