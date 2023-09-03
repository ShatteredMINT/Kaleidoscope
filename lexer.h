#pragma once

#include <string>

enum Token {
    tok_eof = -1,

    //command
    tok_def = -2,
    tok_extern =-3,

    //primary expression
    tok_identifier = -4,
    tok_number = -5,

    //control
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,
};

namespace Lexer {
    extern std::string IdentifierStr;
    extern double NumVal;
    extern int CurTok;

    int getNextToken();

    int gettok();;
};
