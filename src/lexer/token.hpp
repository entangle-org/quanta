#pragma once

#include "token_type.hpp"
#include <string>

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};