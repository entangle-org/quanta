#include "parser.hpp"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens)
    , current(0) {}

std::shared_ptr<Program> Parser::parse() {
    auto program = std::make_shared<Program>();
    while (!check(TokenType::Eof)) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::advance() {
    if (!check(TokenType::Eof))
        current++;
    return previous();
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

const Token& Parser::consumeOrError(TokenType type, const std::string& message) {
    if (check(type))
        return advance();
    std::cerr << "[Parser Error] Line " << peek().line << ", Col " << peek().column << ": " << message << "\n";
    std::exit(1);
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

std::shared_ptr<Statement> Parser::parseStatement() {
    if (check(TokenType::Quantum) || check(TokenType::Classical)) {
        return parseFunctionDecl();
    }

    if (isPrimitive(peek().type)) {
        return parseVariableDecl();
    }

    return parseExpressionStmt();
}

std::shared_ptr<Statement> Parser::parseFunctionDecl() {
    bool isQuantum = match(TokenType::Quantum);
    if (!isQuantum)
        consumeOrError(TokenType::Classical, "Expected 'quantum' or 'classical'");

    consumeOrError(TokenType::Function, "Expected 'function'");
    std::string name = consumeOrError(TokenType::Identifier, "Expected function name").value;
    consumeOrError(TokenType::LParen, "Expected '(' after function name");

    std::vector<Parameter> params;
    if (!check(TokenType::RParen)) {
        do {
            Token primitiveToken = peek();
            if (!isPrimitive(primitiveToken.type)) {
                std::cerr << "[Parser Error] Line " << primitiveToken.line << ", Col " << primitiveToken.column
                          << ": Expected parameter type\n";
                std::exit(1);
            }
            advance();
            std::string type = primitiveToken.value;
            std::string paramName = consumeOrError(TokenType::Identifier, "Expected parameter name").value;
            params.push_back({type, paramName});
        } while (match(TokenType::Comma));
    }

    consumeOrError(TokenType::RParen, "Expected ')' after parameters");
    consumeOrError(TokenType::Arrow, "Expected '->'");
    Token returnToken = peek();
    if (!isPrimitive(returnToken.type) && returnToken.type != TokenType::Void) {
        std::cerr << "[Parser Error] Line " << returnToken.line << ", Col " << returnToken.column << ": Expected return type\n";
        std::exit(1);
    }
    advance();
    std::string returnType = returnToken.value;

    consumeOrError(TokenType::LBrace, "Expected '{' before function body");

    auto func = std::make_shared<FunctionDecl>(isQuantum, name, params, returnType);

    while (!check(TokenType::RBrace)) {
        func->body.push_back(parseStatement());
    }

    consumeOrError(TokenType::RBrace, "Expected '}' after function body");
    return func;
}

std::shared_ptr<Statement> Parser::parseVariableDecl() {
    std::string typeName = advance().value;
    std::shared_ptr<Expression> initializer = nullptr;

    if (typeName == "qubit" && check(TokenType::QubitLiteral)) {
        initializer = std::make_shared<LiteralExpr>(advance().value);
    }

    Token token = consumeOrError(TokenType::Identifier, "Expected variable name");

    if (match(TokenType::Equals)) {
        initializer = parseExpression();
    }

    consumeOrError(TokenType::Semicolon, "Expected ';' after variable declaration");
    return std::make_shared<VariableDecl>(typeName, token.value, initializer);
}

std::shared_ptr<Statement> Parser::parseExpressionStmt() {
    auto expr = parseExpression();
    consumeOrError(TokenType::Semicolon, "Expected ';' after expression");
    return std::make_shared<ExpressionStmt>(expr);
}

std::shared_ptr<Expression> Parser::parseExpression() {
    auto expr = parsePrimary();

    if (match(TokenType::Equals)) {
        if (auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
            auto value = parseExpression();
            return std::make_shared<AssignmentExpr>(id->name, value);
        } else {
            std::cerr << "[Parser Error] Invalid assignment target at line " << previous().line << "\n";
            std::exit(1);
        }
    }

    return expr;
}

std::shared_ptr<Expression> Parser::parsePrimary() {
    if (match(TokenType::IntegerLiteral) || match(TokenType::FloatLiteral) || match(TokenType::StringLiteral) ||
        match(TokenType::CharLiteral)) {
        return std::make_shared<LiteralExpr>(previous().value);
    }

    if (match(TokenType::Identifier)) {
        std::string name = previous().value;
        if (check(TokenType::LParen)) {
            return parseCall(std::make_shared<IdentifierExpr>(name));
        }
        return std::make_shared<IdentifierExpr>(name);
    }

    if (match(TokenType::Measure)) {
        return parseMeasure();
    }

    std::cerr << "[Parser Error] Unexpected token '" << peek().value << "' at line " << peek().line << "\n";
    std::exit(1);
}

std::shared_ptr<Expression> Parser::parseCall(std::shared_ptr<Expression> callee) {
    consumeOrError(TokenType::LParen, "Expected '(' after function name");
    std::vector<std::shared_ptr<Expression>> args;
    if (!check(TokenType::RParen)) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }
    consumeOrError(TokenType::RParen, "Expected ')' after arguments");
    return std::make_shared<CallExpr>(dynamic_cast<IdentifierExpr&>(*callee).name, args);
}

std::shared_ptr<Expression> Parser::parseMeasure() {
    auto target = parsePrimary();
    return std::make_shared<CallExpr>("measure", std::vector<std::shared_ptr<Expression>>{target});
}

bool Parser::isPrimitive(TokenType type) const {
    return type == TokenType::Int || type == TokenType::Float || type == TokenType::Qubit || type == TokenType::Char ||
           type == TokenType::String;
}
