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

    std::vector<Annotation> annotations;
    if (check(TokenType::At)) {
        annotations = parseAnnotations();
    }

    if (check(TokenType::Function)) {
        return parseFunctionDecl(annotations);
    }

    if (check(TokenType::Final) || isPrimitive(peek().type)) {
        return parseVariableDecl(annotations);
    }

    if (!annotations.empty()) {
        error("Unexpected annotations on non-declarative statement", peek().line, peek().column);
    }

    return parseExpressionStmt();
}

std::shared_ptr<Statement> Parser::parseFunctionDecl(const std::vector<Annotation>& annotations) {
    bool isQuantum = false;
    bool isAdjoint = false;

    for (const auto& annotation : annotations) {
        if (annotation.name == "quantum") {
            isQuantum = true;
        } else if (annotation.name == "adjoint") {
            isAdjoint = true;
        } else {
            error("Unknown annotation @" + annotation.name, peek().line, peek().column);
        }
    }

    if (isAdjoint && !isQuantum) {
        error("@adjoint annotation is only allowed on @quantum functions", peek().line, peek().column);
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

std::shared_ptr<Statement> Parser::parseVariableDecl(const std::vector<Annotation>& annotations) {
    std::shared_ptr<Expression> initialiser = nullptr;
    bool isFinal = false;
    bool isArray = false;
    int arraySize = 0;

    if (!annotations.empty()) {
        for (const auto& annotation : annotations) {
            if (annotation.name == "state") {
                if (annotation.argument.empty()) {
                    error("@state must have a char literal argument", peek().line, peek().column);
                }
                initialiser = std::make_shared<LiteralExpr>(annotation.argument);
            } else {
                error("Unknown annotation @" + annotation.name, peek().line, peek().column);
            }
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

std::vector<Annotation> Parser::parseAnnotations() {
    std::vector<Annotation> annotations;
    while (match(TokenType::At)) {
        annotations.push_back(parseSingleAnnotation());
    }
    return annotations;
}

Annotation Parser::parseSingleAnnotation() {
    Token ident = consumeOrError(TokenType::Identifier, "Expected annotation name after '@'");

    std::string name = ident.value;
    std::string argument;

    if (match(TokenType::LParen)) {
        if (check(TokenType::CharLiteral)) {
            Token arg = advance();
            argument = arg.value;
        } else {
            error("Only char literals allowed as annotation arguments", peek().line, peek().column);
        }
        consumeOrError(TokenType::RParen, "Expected ')' after annotation argument");
    }

    return Annotation(name, argument);
}
