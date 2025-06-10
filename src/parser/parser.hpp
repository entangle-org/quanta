#pragma once

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::shared_ptr<Program> parse();

private:
    const std::vector<Token>& tokens;
    size_t current;

    const Token& peek() const;
    const Token& advance();
    const Token& previous() const;
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isPrimitive(TokenType type) const;
    const Token& consumeOrError(TokenType type, const std::string& message);

    std::shared_ptr<Statement> parseStatement();
    std::shared_ptr<Statement> parseFunctionDecl();
    std::shared_ptr<Statement> parseVariableDecl();
    std::shared_ptr<Statement> parseExpressionStmt();

    std::shared_ptr<Expression> parseExpression();
    std::shared_ptr<Expression> parsePrimary();
    std::shared_ptr<Expression> parseCall(std::shared_ptr<Expression> callee);
    std::shared_ptr<Expression> parseMeasure();
};