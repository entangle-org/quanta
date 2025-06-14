#pragma once
#include <string>

enum class TokenType {
    // Literals
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,

    // Keywords
    Int,
    Float,
    String,
    Char,
    Qubit,
    Bit,
    Void,
    Function,
    Import,
    Return,
    If,
    For,
    Class,
    Measure,
    Final,
    Reset,

    // Symbols and operators
    Equals,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Semicolon,
    Comma,
    Dot,
    Colon,
    Arrow,
    At,
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,

    // Control
    Eof,
    Unknown
};