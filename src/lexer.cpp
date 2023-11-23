#include "include/lexer.h"

#include <cstdio>
#include <string>

// "initialize"
std::string Lexer::IdentifierStr;
double Lexer::NumVal;
int Lexer::CurTok;
FILE *Lexer::stream = stdin;

int Lexer::gettok() {
  static int LastChar = ' ';

  while (isspace(LastChar))
    LastChar = getc(stream);

  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;

    while (isalnum(LastChar = getc(stream)))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
      return tok_extern;
    if (IdentifierStr == "if")
      return tok_if;
    if (IdentifierStr == "then")
      return tok_then;
    if (IdentifierStr == "else")
      return tok_else;
    if (IdentifierStr == "for")
      return tok_for;
    if (IdentifierStr == "in")
      return tok_in;
    if (IdentifierStr == "binary")
      return tok_binary;
    if (IdentifierStr == "unary")
      return tok_unary;
    if (IdentifierStr == "var")
      return tok_var;

    return tok_identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getc(stream);
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), 0);
    return tok_number;
  }

  // handle comments
  if (LastChar == '#') {
    do
      LastChar = getc(stream);
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }

  if (LastChar == EOF) {
    LastChar = ' ';
    return tok_eof;
  }

  int ThisChar = LastChar;
  LastChar = getc(stream);
  return ThisChar;
}

int Lexer::getNextToken() { return CurTok = gettok(); }
