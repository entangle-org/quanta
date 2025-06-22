#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"

class Parser {
public:
  explicit Parser(const std::vector<Token> &tokens);
  std::shared_ptr<Program> parse();

private:
  const std::vector<Token> &tokens;
  size_t current;

  const Token &peek() const;
  const Token &advance();
  const Token &previous() const;
  bool match(TokenType type);
  bool check(TokenType type) const;
  bool isPrimitive(TokenType type) const;

  const Token &consumeOrError(TokenType type, const std::string &message);
  const void error(const std::string &message, const int &line,
                   const int &column);

  std::shared_ptr<Statement> parseStatement();
  std::shared_ptr<Statement>
  parseFunctionDecl(const std::vector<Annotation> &annotations);
  std::shared_ptr<Statement>
  parseVariableDecl(const std::vector<Annotation> &annotations);
  std::shared_ptr<Statement> parseExpressionStmt();
  std::shared_ptr<Statement> parseResetStmt();
  std::shared_ptr<Statement> parseAdjointStmt();
  std::shared_ptr<Statement> parseReturnStmt();

  std::vector<Annotation> parseAnnotations();
  Annotation parseSingleAnnotation();

  std::shared_ptr<Expression> parseExpression();
  std::shared_ptr<Expression> parsePrimary();
  std::shared_ptr<Expression> parseCall(std::shared_ptr<Expression> callee);
  std::shared_ptr<Expression> parseMeasure();
};