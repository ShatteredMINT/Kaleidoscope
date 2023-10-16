#pragma once

#include <string>
#include <cstdio>

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
    tok_for = -9,
    tok_in = -10,

    // operators
    tok_binary = -11,
    tok_unary = -12
};

namespace Lexer {
    /**contains currently parsed string*/
    extern std::string IdentifierStr;
    /**numerical value of current string*/
    extern double NumVal;
    /**current token*/
    extern int CurTok;

    /** stream from which to read input **/
    extern FILE * stream;

    /**helper to grab current token and store it*/
    int getNextToken();

    int gettok();;
};
