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
};

class Lexer {

public:
    static std::string IdentifierStr;
    static double NumVal;
    static int CurTok;


    static int getNextToken();

    static int gettok();;
};
