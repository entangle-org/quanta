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
const Token& Parser::previous() const {
    return tokens[current - 1];
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

const void Parser::error(const std::string& message, const int& line, const int& column) {
    std::cerr << "[Parser Error] Line " << line << ", Col " << column << ": " << message << "\n";
    std::exit(1);
}

bool Parser::isPrimitive(TokenType type) const {
    return type == TokenType::Int || type == TokenType::Float || type == TokenType::Qubit || type == TokenType::Char ||
           type == TokenType::String || type == TokenType::Bit;
}

std::shared_ptr<Statement> Parser::parseStatement() {
    if (match(TokenType::Return))
        return parseReturnStmt();
    if (match(TokenType::Reset))
        return parseResetStmt();
    if (check(TokenType::At)) {

        if (tokens.size() <= current + 1) {
            error("Unexpected end after '@'", peek().line, peek().column);
        }

        Token next = tokens[current + 1];
        const std::string& annotation = next.value;

        if (annotation == "adjoint") {
            return parseAdjointStmt();
        } else if (annotation == "quantum") {
            return parseFunctionDecl();
        } else if (annotation == "state") {
            return parseVariableDecl();
        } else {
            error("Unknown annotation @" + annotation, next.line, next.column);
        }
    }
    if (check(TokenType::Function))
        return parseFunctionDecl();
    if (check(TokenType::Final) || isPrimitive(peek().type))
        return parseVariableDecl();

    return parseExpressionStmt();
}

std::shared_ptr<Statement> Parser::parseFunctionDecl() {
    bool isQuantum = false;
    bool isAdjoint = false;

    while (match(TokenType::At)) {
        Token annotation = consumeOrError(TokenType::Identifier, "Expected annotation name");

        if (annotation.value == "quantum") {
            isQuantum = true;
        } else if (annotation.value == "adjoint") {
            isAdjoint = true;
        } else {
            error("Unknown annotation @" + annotation.value, annotation.line, annotation.column);
        }
    }

    if (isAdjoint && !isQuantum) {
        std::cerr << "[Parser Error] @adjoint annotation is only allowed on @quantum functions\n";
        std::exit(1);
    }

    consumeOrError(TokenType::Function, "Expected 'function'");
    std::string name = consumeOrError(TokenType::Identifier, "Expected function name").value;
    consumeOrError(TokenType::LParen, "Expected '('");

    std::vector<Parameter> params;
    if (!check(TokenType::RParen)) {
        do {
            std::string type = consumeOrError(peek().type, "Expected parameter type").value;
            std::string name = consumeOrError(TokenType::Identifier, "Expected parameter name").value;
            params.push_back({type, name});
        } while (match(TokenType::Comma));
    }

    consumeOrError(TokenType::RParen, "Expected ')'");
    consumeOrError(TokenType::Arrow, "Expected '->'");
    std::string returnType = consumeOrError(peek().type, "Expected return type").value;
    if (isQuantum && !(returnType == "void" || returnType == "bit")) {
        error("@quantum functions may only return 'void' or 'bit'", peek().line, peek().column);
    }
    consumeOrError(TokenType::LBrace, "Expected '{'");

    auto func = std::make_shared<FunctionDecl>(isQuantum, name, params, returnType);
    func->isAdjoint = isAdjoint;

    while (!check(TokenType::RBrace)) {
        func->body.push_back(parseStatement());
    }

    consumeOrError(TokenType::RBrace, "Expected '}'");
    return func;
}

std::shared_ptr<Statement> Parser::parseVariableDecl() {
    std::shared_ptr<Expression> initialiser = nullptr;
    bool isFinal = false;
    bool isArray = false;
    int arraySize = 0;

    if (match(TokenType::At)) {
        Token annotation = consumeOrError(TokenType::Identifier, "Expected annotation name after '@'");
        if (annotation.value == "state") {
            consumeOrError(TokenType::LParen, "Expected '(' after @state");
            Token stateToken = consumeOrError(TokenType::CharLiteral,
                                              "[Parser Error] Use single quotes in @state (e.g. @state('+'), not @state(\"+\"))\n");
            consumeOrError(TokenType::RParen, "Expected ')' after @state string");
            initialiser = std::make_shared<LiteralExpr>(stateToken.value);
        } else {
            error("Unknown annotation @" + annotation.value, annotation.line, annotation.column);
        }
    }

    if (match(TokenType::Final))
        isFinal = true;

    std::string primitive = consumeOrError(peek().type, "Expected type").value;

    if (match(TokenType::LBracket)) {
        arraySize = std::stoi(consumeOrError(TokenType::IntegerLiteral, "Expected array size").value);
        consumeOrError(TokenType::RBracket, "Expected ']'");
        isArray = true;
    }

    std::string name = consumeOrError(TokenType::Identifier, "Expected variable name").value;

    if (match(TokenType::Equals)) {
        initialiser = parseExpression();
    }

    consumeOrError(TokenType::Semicolon, "Expected ';'");

    auto decl = std::make_shared<VariableDecl>(primitive, name, initialiser);
    decl->isFinal = isFinal;
    decl->isArray = isArray;
    decl->arraySize = arraySize;
    return decl;
}

std::shared_ptr<Statement> Parser::parseExpressionStmt() {
    auto expr = parseExpression();
    consumeOrError(TokenType::Semicolon, "Expected ';'");
    return std::make_shared<ExpressionStmt>(expr);
}

std::shared_ptr<Statement> Parser::parseResetStmt() {
    auto target = parseExpression();
    consumeOrError(TokenType::Semicolon, "Expected ';'");
    return std::make_shared<ResetStmt>(target);
}

std::shared_ptr<Statement> Parser::parseAdjointStmt() {
    consumeOrError(TokenType::At, "Expected '@'");
    Token keyword = consumeOrError(TokenType::Identifier, "Expected 'adjoint'");
    if (keyword.value != "adjoint") {
        error("Unknown annotation @" + keyword.value, keyword.line, keyword.column);
    }

    std::string callee = consumeOrError(TokenType::Identifier, "Expected function name").value;
    consumeOrError(TokenType::LParen, "Expected '('");

    std::vector<std::shared_ptr<Expression>> args;
    if (!check(TokenType::RParen)) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }

    consumeOrError(TokenType::RParen, "Expected ')'");
    consumeOrError(TokenType::Semicolon, "Expected ';'");

    return std::make_shared<AdjointStmt>(callee, args);
}

std::shared_ptr<Statement> Parser::parseReturnStmt() {
    auto expr = parseExpression();
    consumeOrError(TokenType::Semicolon, "Expected ';'");
    return std::make_shared<ReturnStmt>(expr);
}

std::shared_ptr<Expression> Parser::parseExpression() {
    auto expr = parsePrimary();

    if (match(TokenType::Equals)) {
        if (auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
            auto value = parseExpression();
            return std::make_shared<AssignmentExpr>(id->name, value);
        } else {
            error("Invalid assignment target", previous().line, previous().column);
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
        auto target = parsePrimary();
        return std::make_shared<CallExpr>("measure", std::vector<std::shared_ptr<Expression>>{target});
    }

    error("Unexpected token " + peek().value, peek().line, peek().column);
}

std::shared_ptr<Expression> Parser::parseCall(std::shared_ptr<Expression> callee) {
    consumeOrError(TokenType::LParen, "Expected '('");
    std::vector<std::shared_ptr<Expression>> args;
    if (!check(TokenType::RParen)) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }
    consumeOrError(TokenType::RParen, "Expected ')'");
    return std::make_shared<CallExpr>(dynamic_cast<IdentifierExpr&>(*callee).name, args);
}
