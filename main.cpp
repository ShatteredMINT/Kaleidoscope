#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>



//===-----------------------------------------------------------------
// LEXER
//===-----------------------------------------------------------------

enum Token {
    tok_eof = -1,

    //command
    tok_def = -2,
    tok_extern =-3,

    //primary expression
    tok_identifier = -4,
    tok_number = -5,
};

static std::string IdentiefierStr;
static double NumVal;

static int gettok() {
    static int LastChar = ' ';

    //skip whitespace
    while (isspace(LastChar))
        LastChar = getchar();

    if (isalpha(LastChar)) {
        IdentiefierStr = LastChar;
        while (isalnum((LastChar = getchar())))
            IdentiefierStr += LastChar;
        if (IdentiefierStr == "def")
            return tok_def;
        if (IdentiefierStr == "extern")
            return tok_extern;

        return tok_identifier;
    }

    if (isdigit(LastChar) || LastChar == '.') {
        std:: string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar || LastChar == '.'));

        NumVal = strtod(NumStr.c_str(), 0);
        return tok_number;
    }

    if (LastChar == '#') {
        do
            LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }

    if (LastChar == EOF)
        return tok_eof;

    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}


int main() {
    while (true)
        printf("%d", gettok());
}
