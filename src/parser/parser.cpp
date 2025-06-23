#include "parser.hpp"
#include <iostream>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

// Token manipulation

const Token &Parser::peek() const {
  if (current >= tokens.size())
    return tokens.back();
  return tokens[current];
}
const Token &Parser::previous() const { return tokens[current - 1]; }
const Token &Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}
const Token &Parser::expect(TokenType type, const std::string &message) {
  if (check(type))
    return advance();
  reportError(message);
  return peek();
}

// Token matching
bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}
bool Parser::check(TokenType type) const {
  if (isAtEnd())
    return false;
  return peek().type == type;
}
bool Parser::checkNext(TokenType type) const {
  if (current + 1 >= tokens.size())
    return false;
  return tokens[current + 1].type == type;
}
bool Parser::checkFunctionAnnotation() const {
  if (!check(TokenType::At))
    return false;
  if (!checkNext(TokenType::Quantum) && !checkNext(TokenType::Adjoint))
    return false;
  return true;
}

bool Parser::isAtEnd() const { return peek().type == TokenType::Eof; }

// Error
void Parser::reportError(const std::string &message) {
  const Token &token = peek();
  std::cerr << "[Quanta Parser Error]"
            << "\n"
            << "Line " << token.line << ", Col " << token.column
            << ", Token = '" << token.value << "'\n"
            << message << "\n";
  std::exit(1);
}

// Main parse function
std::unique_ptr<Program> Parser::parse() {
  auto program = std::make_unique<Program>();

  while (!isAtEnd()) {
    if (check(TokenType::Import)) {
      program->imports.push_back(parseImport());
    } else if (check(TokenType::Function) || checkFunctionAnnotation()) {
      program->functions.push_back(parseFunction());
    } else if (check(TokenType::Class)) {
      program->classes.push_back(parseClass());
    } else {
      program->statements.push_back(parseTopLevelStatement());
    }
  }

  return program;
}

// Top level

// import module;
std::unique_ptr<ImportStatement> Parser::parseImport() {
  expect(TokenType::Import, "Expected 'import' keyword");

  auto stmt = std::make_unique<ImportStatement>();
  if (!check(TokenType::Identifier)) {
    reportError("Expected module name after 'import'");
  }

  stmt->module = advance().value;
  expect(TokenType::Semicolon, "Expected ';' after import statement");

  return stmt;
}

// Function Declaration
std::unique_ptr<FunctionDeclaration> Parser::parseFunction() {
  auto func = std::make_unique<FunctionDeclaration>();

  // Parse annotations
  while (check(TokenType::At)) {
    advance();
    if (match(TokenType::Quantum) || match(TokenType::Adjoint)) {
      std::string name = previous().value;
      std::string value = "";
      func->annotations.push_back(
          std::make_unique<AnnotationNode>(AnnotationNode{name, value}));
      func->hasQuantumAnnotation = true;
    } else {
      reportError("Expected annotation name after '@'");
    }
  }

  expect(TokenType::Function, "Expected 'function' keyword");

  // Parse function name
  if (!check(TokenType::Identifier)) {
    reportError("Expected function name after 'function' keyword");
  }
  func->name = advance().value;

  // Parse parameters
  expect(TokenType::LParen, "Expected '(' after function name");
  while (!check(TokenType::RParen)) {
    auto param = std::make_unique<Parameter>();

    param->type = parseType(); // <- use full type parser

    if (!check(TokenType::Identifier)) {
      reportError("Expected parameter name");
    }
    param->name = advance().value;

    func->params.push_back(std::move(param));

    if (!match(TokenType::Comma))
      break;
  }
  expect(TokenType::RParen, "Expected ')' after parameters");

  // Return type
  expect(TokenType::Arrow, "Expected '->' before return type");

  func->returnType = parseType();

  // Body
  func->body = parseBlock();

  return func;
}

std::unique_ptr<ClassDeclaration> Parser::parseClass() {
  expect(TokenType::Class, "Expected 'class' keyword");

  auto clazz = std::make_unique<ClassDeclaration>();

  if (!check(TokenType::Identifier)) {
    reportError("Expected class name after 'class'");
  }
  clazz->name = advance().value;

  expect(TokenType::LBrace, "Expected '{' to start class body");

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    // Handle @members("public") or @members("private")
    if (check(TokenType::At) && checkNext(TokenType::Members)) {
      advance();
      advance();
      expect(TokenType::LParen, "Expected '(' after @members");

      if (!check(TokenType::StringLiteral)) {
        reportError("Expected access modifier string in @members");
      }
      std::string accessModifier = advance().value;
      if (accessModifier != "\"public\"" && accessModifier != "\"private\"") {
        reportError(
            "Invalid access modifier: must be \"public\" or \"private\"");
      }
      accessModifier = accessModifier.substr(1, accessModifier.length() - 2);

      expect(TokenType::RParen, "Expected ')' after access modifier");

      // Parse variable declarations inside the block
      while (!check(TokenType::At) && !check(TokenType::RBrace)) {
        bool isFinal = match(TokenType::Final);
        auto member = parseVariableDeclaration(isFinal);

        // Annotate with access level
        member->access = accessModifier;
        clazz->members.push_back(std::move(member));
      }

    } else if (check(TokenType::At) && checkNext(TokenType::Methods)) {
      advance();
      advance();
      // Everything after this point must be functions
      while (!check(TokenType::RBrace) && !isAtEnd()) {
        clazz->methods.push_back(parseFunction());
      }
    } else {
      reportError("Only @members or @methods are allowed inside class body");
    }
  }

  expect(TokenType::RBrace, "Expected '}' to end class body");

  return clazz;
}

std::unique_ptr<Statement> Parser::parseTopLevelStatement() {
  bool isFinal = match(TokenType::Final);

  if (check(TokenType::At) || check(TokenType::Int) ||
      check(TokenType::Float) || check(TokenType::String) ||
      check(TokenType::Char) || check(TokenType::Bit) ||
      check(TokenType::Qubit)) {
    return parseVariableDeclaration(isFinal);
  }

  // Fallback to expression statement
  auto exprStmt = std::make_unique<ExpressionStatement>();
  exprStmt->expression = parseExpression();
  expect(TokenType::Semicolon, "Expected ';' after expression");
  return exprStmt;
}

// Declarations
std::unique_ptr<VariableDeclaration>
Parser::parseVariableDeclaration(bool isFinal) {
  auto var = std::make_unique<VariableDeclaration>();
  var->isFinal = isFinal;

  // Annotations
  var->annotations = parseAnnotations();

  // Type
  var->varType = parseType();

  // Name
  if (!check(TokenType::Identifier)) {
    reportError("Expected variable name");
  }
  var->name = advance().value;

  // Initializer
  if (match(TokenType::Equals)) {
    var->initializer = parseExpression();
  }

  expect(TokenType::Semicolon, "Expected ';' after variable declaration");

  return var;
}

// @quantum, @adjoint, @state
std::unique_ptr<AnnotationNode> Parser::parseAnnotation() {
  expect(TokenType::At, "Expected '@' to begin annotation");

  if (!check(TokenType::Quantum) && !check(TokenType::Adjoint) &&
      !check(TokenType::State)) {
    reportError("Unknown annotation");
  }

  auto nameToken = advance();
  auto annotation = std::make_unique<AnnotationNode>();
  annotation->name = nameToken.value;

  if (annotation->name == "state") {
    expect(TokenType::LParen, "Expected '(' after @state");
    if (!check(TokenType::CharLiteral) && !check(TokenType::StringLiteral)) {
      reportError("Expected character or string inside @state(...)");
    }
    annotation->value = advance().value;
    expect(TokenType::RParen, "Expected ')' after @state argument");
  }

  return annotation;
}

std::vector<std::unique_ptr<AnnotationNode>> Parser::parseAnnotations() {
  std::vector<std::unique_ptr<AnnotationNode>> annotations;

  while (check(TokenType::At)) {
    annotations.push_back(parseAnnotation());
  }

  return annotations;
}

// Statements
std::unique_ptr<Statement> Parser::parseStatement() {
  bool isFinal = match(TokenType::Final);

  // Variable declarations inside blocks
  if (check(TokenType::Int) || check(TokenType::Float) ||
      check(TokenType::String) || check(TokenType::Char) ||
      check(TokenType::Bit) || check(TokenType::Qubit)) {
    return parseVariableDeclaration(isFinal);
  }

  if (match(TokenType::Return))
    return parseReturn();
  if (match(TokenType::If))
    return parseIf();
  if (match(TokenType::For))
    return parseFor();
  if (match(TokenType::Echo))
    return parseEcho();
  if (match(TokenType::Reset))
    return parseReset();
  if (match(TokenType::Measure))
    return parseMeasure();
  if (check(TokenType::Identifier) && checkNext(TokenType::Equals))
    return parseAssignment();
  return parseExpressionStatement();
}

// {...}
std::unique_ptr<BlockStatement> Parser::parseBlock() {
  expect(TokenType::LBrace, "Expected '{' to start block");

  auto block = std::make_unique<BlockStatement>();
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    block->statements.push_back(parseStatement());
  }

  expect(TokenType::RBrace, "Expected '}' to end block");
  return block;
}

// return expr;
std::unique_ptr<ReturnStatement> Parser::parseReturn() {
  auto stmt = std::make_unique<ReturnStatement>();

  if (!check(TokenType::Semicolon)) {
    stmt->value = parseExpression();
  }

  expect(TokenType::Semicolon, "Expected ';' after return value");
  return stmt;
}

// if (cond) {...}
// NOTE: while elseBranch field exists, we are setting to nullptr here but this
// can be extended in future
std::unique_ptr<IfStatement> Parser::parseIf() {
  expect(TokenType::LParen, "Expected '(' after 'if'");
  auto condition = parseExpression();
  expect(TokenType::RParen, "Expected ')' after condition");

  auto thenBranch = parseBlock();

  auto stmt = std::make_unique<IfStatement>();
  stmt->condition = std::move(condition);
  stmt->thenBranch = std::move(thenBranch);
  stmt->elseBranch = nullptr;

  return stmt;
}

// for (init; cond; update) {...}
std::unique_ptr<ForStatement> Parser::parseFor() {
  expect(TokenType::LParen, "Expected '(' after 'for'");

  std::unique_ptr<Statement> initializer = nullptr;

  if (!check(TokenType::Semicolon)) {
    bool isFinal = match(TokenType::Final);

    if (check(TokenType::Int) || check(TokenType::Float) ||
        check(TokenType::Char) || check(TokenType::String) ||
        check(TokenType::Bit) || check(TokenType::Qubit)) {
      initializer = parseVariableDeclaration(isFinal);
    } else {
      if (isFinal) {
        reportError("Expected variable type after 'final'");
      }
      initializer = parseExpressionStatement();
    }
  } else {
    advance();
  }

  auto condition = parseExpression();
  expect(TokenType::Semicolon, "Expected ';' after loop condition");

  auto increment = parseExpression();
  expect(TokenType::RParen, "Expected ')' after for clause");

  auto body = parseBlock();

  auto stmt = std::make_unique<ForStatement>();
  stmt->initializer = std::move(initializer);
  stmt->condition = std::move(condition);
  stmt->increment = std::move(increment);
  stmt->body = std::move(body);

  return stmt;
}

// echo(expr);
std::unique_ptr<EchoStatement> Parser::parseEcho() {
  expect(TokenType::LParen, "Expected '(' after 'echo'");
  auto value = parseExpression();
  expect(TokenType::RParen, "Expected ')' after echo argument");
  expect(TokenType::Semicolon, "Expected ';' after echo statement");

  auto stmt = std::make_unique<EchoStatement>();
  stmt->value = std::move(value);
  return stmt;
}

// reset q0;
std::unique_ptr<ResetStatement> Parser::parseReset() {
  auto stmt = std::make_unique<ResetStatement>();
  stmt->target = parseExpression();
  expect(TokenType::Semicolon, "Expected ';' after reset target");
  return stmt;
}

// measure q0;
std::unique_ptr<MeasureStatement> Parser::parseMeasure() {
  auto stmt = std::make_unique<MeasureStatement>();
  stmt->qubit = parseExpression();
  expect(TokenType::Semicolon, "Expected ';' after measure target");
  return stmt;
}

// x = expr;
std::unique_ptr<AssignmentStatement> Parser::parseAssignment() {
  if (!check(TokenType::Identifier)) {
    reportError("Expected variable name in assignment");
  }

  std::string name = advance().value;
  expect(TokenType::Equals, "Expected '=' in assignment");

  auto stmt = std::make_unique<AssignmentStatement>();
  stmt->name = name;
  stmt->value = parseExpression();

  expect(TokenType::Semicolon, "Expected ';' after assignment");
  return stmt;
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
  auto expr = parseExpression();
  expect(TokenType::Semicolon, "Expected ';' after expression");
  auto stmt = std::make_unique<ExpressionStatement>();
  stmt->expression = std::move(expr);
  return stmt;
}

// Expressions

std::unique_ptr<Expression> Parser::parseExpression() {
  return parseAssignmentExpression();
}

std::unique_ptr<Expression> Parser::parseAssignmentExpression() {
  auto expr = parseComparison();

  if (match(TokenType::Equals)) {
    auto equals = previous();

    // Must be a variable on the left-hand side
    if (auto varExpr = dynamic_cast<VariableExpression *>(expr.get())) {
      std::string name = varExpr->name;
      auto value = parseAssignmentExpression();
      return std::make_unique<AssignmentExpression>(name, std::move(value));
    } else {
      reportError("Invalid assignment target");
    }
  }

  return expr;
}

std::unique_ptr<Expression> Parser::parseComparison() {
  auto expr = parseAdditive();

  while (match(TokenType::Greater) || match(TokenType::Less) ||
         match(TokenType::GreaterEqual) || match(TokenType::LessEqual)) {
    std::string op = previous().value;
    auto right = parseAdditive();
    expr = std::make_unique<BinaryExpression>(
        BinaryExpression{op, std::move(expr), std::move(right)});
  }

  return expr;
}

std::unique_ptr<Expression> Parser::parseAdditive() {
  auto expr = parseMultiplicative();

  while (match(TokenType::Plus) || match(TokenType::Minus)) {
    std::string op = previous().value;
    auto right = parseMultiplicative();
    expr = std::make_unique<BinaryExpression>(
        BinaryExpression{op, std::move(expr), std::move(right)});
  }

  return expr;
}

std::unique_ptr<Expression> Parser::parseMultiplicative() {
  auto expr = parseUnary();

  while (match(TokenType::Star) || match(TokenType::Slash) ||
         match(TokenType::Percent)) {
    std::string op = previous().value;
    auto right = parseUnary();
    expr = std::make_unique<BinaryExpression>(
        BinaryExpression{op, std::move(expr), std::move(right)});
  }

  return expr;
}

std::unique_ptr<Expression> Parser::parseUnary() {
  if (match(TokenType::Minus)) {
    std::string op = previous().value;
    auto right = parseUnary();
    return std::make_unique<UnaryExpression>(
        UnaryExpression{op, std::move(right)});
  }

  return parseCall();
}

std::unique_ptr<Expression> Parser::parseCall() {
  auto expr = parsePrimary();

  while (match(TokenType::LParen)) {
    auto args = parseArgumentList();
    auto *varExpr = dynamic_cast<VariableExpression *>(expr.get());
    if (!varExpr) {
      reportError("Only variables can be used as function names in calls");
    }
    expr = std::make_unique<CallExpression>(varExpr->name, std::move(args));
  }

  return expr;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
  if (match(TokenType::IntegerLiteral) || match(TokenType::FloatLiteral) ||
      match(TokenType::StringLiteral) || match(TokenType::CharLiteral)) {
    return std::make_unique<LiteralExpression>(
        LiteralExpression{previous().value});
  }

  if (match(TokenType::Measure)) {
    auto target = parseExpression();
    return std::make_unique<MeasureExpression>(
        MeasureExpression{std::move(target)});
  }

  if (match(TokenType::Identifier)) {
    return std::make_unique<VariableExpression>(
        VariableExpression{previous().value});
  }

  if (match(TokenType::LParen)) {
    auto expr = parseExpression();
    expect(TokenType::RParen, "Expected ')' after expression");
    return std::make_unique<ParenthesizedExpression>(
        ParenthesizedExpression{std::move(expr)});
  }

  reportError("Expected expression");
  return nullptr;
}

// Literals
std::unique_ptr<Expression> Parser::parseLiteral() {
  const Token &token = advance();

  switch (token.type) {
  case TokenType::IntegerLiteral:
    return std::make_unique<LiteralExpression>(LiteralExpression{token.value});
  case TokenType::FloatLiteral:
    return std::make_unique<LiteralExpression>(LiteralExpression{token.value});
  case TokenType::CharLiteral:
    return std::make_unique<LiteralExpression>(LiteralExpression{token.value});
  case TokenType::StringLiteral:
    return std::make_unique<LiteralExpression>(LiteralExpression{token.value});
  default:
    reportError("Expected a literal value.");
    return nullptr;
  }
}

// Types

std::unique_ptr<Type> Parser::parseType() {
  auto baseType = parsePrimitiveType();

  if (match(TokenType::LBracket)) {
    expect(TokenType::RBracket, "Expected ']' after '[' in array type");
    return parseArrayType(std::move(baseType));
  }

  return baseType;
}

std::unique_ptr<Type> Parser::parsePrimitiveType() {
  const Token &token = advance();

  if (token.type == TokenType::Identifier) {
    return std::make_unique<PrimitiveType>(PrimitiveType{token.value});
  }

  switch (token.type) {
  case TokenType::Int:
  case TokenType::Float:
  case TokenType::String:
  case TokenType::Char:
  case TokenType::Qubit:
  case TokenType::Bit:
    return std::make_unique<PrimitiveType>(PrimitiveType{token.value});
  case TokenType::Logical:
    return std::make_unique<LogicalType>(LogicalType{token.value});
  case TokenType::Void:
    return std::make_unique<VoidType>();
  default:
    reportError("Expected a valid type.");
    return nullptr;
  }
}

std::unique_ptr<Type>
Parser::parseArrayType(std::unique_ptr<Type> elementType) {
  return std::make_unique<ArrayType>(ArrayType{std::move(elementType)});
}

// Parameters and Argments
std::vector<std::unique_ptr<Parameter>> Parser::parseParameterList() {
  std::vector<std::unique_ptr<Parameter>> parameters;

  while (!check(TokenType::RParen)) {
    auto param = std::make_unique<Parameter>();

    // Parse type
    param->type = parseType();

    // Parse name
    if (!check(TokenType::Identifier)) {
      reportError("Expected parameter name.");
    }
    param->name = advance().value;

    parameters.push_back(std::move(param));

    if (!match(TokenType::Comma))
      break;
  }

  return parameters;
}

std::vector<std::unique_ptr<Expression>> Parser::parseArgumentList() {
  std::vector<std::unique_ptr<Expression>> args;

  if (check(TokenType::RParen)) {
    advance();
    return args;
  }

  do {
    args.push_back(parseExpression());
  } while (match(TokenType::Comma));

  expect(TokenType::RParen, "Expected ')' after argument list");
  return args;
}
