#include "tokenizer.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

Tokenizer::Tokenizer(const std::string& source)
    : source(source)
    , position(0)
    , line(1)
    , column(1) {}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    while (position < source.length()) {
        skipWhitespace();
        if (position < source.length()) {
            tokens.push_back(scanToken());
        }
    }
    tokens.push_back(makeToken(TokenType::Eof, ""));
    return tokens;
}

char Tokenizer::peek() const {
    return position < source.length() ? source[position] : '\0';
}

char Tokenizer::advance() {
    char c = source[position++];
    column++;
    return c;
}

bool Tokenizer::match(char expected) {
    if (position >= source.length() || source[position] != expected) {
        return false;
    }
    position++;
    column++;
    return true;
}

void Tokenizer::skipWhitespace() {
    while (position < source.length()) {
        char c = peek();
        if (isspace(c)) {
            if (c == '\n') {
                line++;
                column = 1;
            }
            advance();
        } else if (c == '#') {
            skipComment();
        } else {
            break;
        }
    }
}

void Tokenizer::skipComment() {
    while (position < source.length() && source[position] != '\n') {
        advance();
    }
}

void Tokenizer::reportError(const std::string& msg) {
    std::cerr << "[Lexer Error] Line " << line << ", Col " << column << ": " << msg << "\n";
    std::exit(1);
}

Token Tokenizer::makeToken(TokenType type, const std::string& value) {
    return Token{type, value, line, column - static_cast<int>(value.length())};
}

Token Tokenizer::scanToken() {
    char c = advance();

    if (isdigit(c))
        return scanNumber();
    if (isalpha(c) || c == '_')
        return scanIdentifierOrKeyword();

    switch (c) {
    case '=':
        return makeToken(TokenType::Equals, "=");
    case '+':
        return makeToken(TokenType::Plus, "+");
    case '-':
        return match('>') ? makeToken(TokenType::Arrow, "->") : makeToken(TokenType::Minus, "-");
    case '*':
        return makeToken(TokenType::Star, "*");
    case '/':
        return makeToken(TokenType::Slash, "/");
    case '%':
        return makeToken(TokenType::Percent, "%");
    case ';':
        return makeToken(TokenType::Semicolon, ";");
    case ',':
        return makeToken(TokenType::Comma, ",");
    case '.':
        return makeToken(TokenType::Dot, ".");
    case ':':
        return makeToken(TokenType::Colon, ":");
    case '@':
        return scanQubitLiteral();
    case '"':
        return scanString();
    case '\'':
        return scanChar();
    case '(':
        return makeToken(TokenType::LParen, "(");
    case ')':
        return makeToken(TokenType::RParen, ")");
    case '{':
        return makeToken(TokenType::LBrace, "{");
    case '}':
        return makeToken(TokenType::RBrace, "}");
    case '[':
        return makeToken(TokenType::LBracket, "[");
    case ']':
        return makeToken(TokenType::RBracket, "]");
    default:
        return makeToken(TokenType::Unknown, std::string(1, c));
    }
}

Token Tokenizer::scanNumber() {
    size_t start = position - 1;
    while (isdigit(peek()))
        advance();

    if (peek() == '.') {
        advance();
        while (isdigit(peek()))
            advance();
        if (peek() == 'f') {
            advance();
            return makeToken(TokenType::FloatLiteral, source.substr(start, position - start));
        } else {
            reportError("Float literal must end with 'f'");
            return makeToken(TokenType::Unknown, source.substr(start, position - start));
        }
    }
    return makeToken(TokenType::IntegerLiteral, source.substr(start, position - start));
}

Token Tokenizer::scanIdentifierOrKeyword() {
    size_t start = position - 1;
    while (isalnum(peek()) || peek() == '_')
        advance();

    std::string text = source.substr(start, position - start);

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int", TokenType::Int},         {"float", TokenType::Float},         {"string", TokenType::String},
        {"char", TokenType::Char},       {"qubit", TokenType::Qubit},         {"void", TokenType::Void},
        {"quantum", TokenType::Quantum}, {"classical", TokenType::Classical}, {"function", TokenType::Function},
        {"import", TokenType::Import},   {"return", TokenType::Return},       {"if", TokenType::If},
        {"for", TokenType::For},         {"class", TokenType::Class},         {"measure", TokenType::Measure}};

    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return makeToken(it->second, text);
    }

    return makeToken(TokenType::Identifier, text);
}

Token Tokenizer::scanString() {
    size_t start = position;
    while (position < source.length() && peek() != '"') {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (peek() == '"') {
        advance();
        return makeToken(TokenType::StringLiteral, source.substr(start - 1, position - start + 1));
    }

    reportError("Unterminated string literal");
    return makeToken(TokenType::Unknown, source.substr(start - 1, position - start + 1));
}

Token Tokenizer::scanChar() {
    size_t start = position;
    if (position < source.length())
        advance();

    if (peek() == '\'') {
        advance();
        return makeToken(TokenType::CharLiteral, source.substr(start - 1, 3));
    }

    reportError("Unterminated char literal");
    return makeToken(TokenType::Unknown, source.substr(start - 1, position - start + 1));
}

Token Tokenizer::scanQubitLiteral() {
    if (!match('(')) {
        return makeToken(TokenType::At, "@");
    }

    size_t start = position;

    // Acceptable characters: + - i 0 1
    while (position < source.length() && (isalnum(peek()) || peek() == '+' || peek() == '-')) {
        advance();
    }

    if (!match(')')) {
        reportError("Unterminated qubit initializer @(...)");
        return makeToken(TokenType::Unknown, source.substr(start - 2, position - start + 2));
    }

    return makeToken(TokenType::QubitLiteral, source.substr(start - 2, position - start + 3));
}
