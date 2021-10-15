#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

using namespace std;

enum CharTypes
{
   CT_EOF,
   CT_SPACE,
   CT_LETTER,
   CT_DIGIT,
   CT_DM,
   CT_ERR,
};

struct Lexeme
{
   int Code{};
   int Line{};
   int Column{};
   string Name{};

   Lexeme() = default;
   explicit Lexeme(const string& name, int code, int line, int column) : Name(name), Code(code), Line(line), Column(column) { }
   explicit Lexeme(const string& name, int code) : Name(name), Code(code) { }
   explicit Lexeme(char c) : Name(string(1, c)), Code(c) { }
};

class Lexer
{
private:
   static bool initedStatic;
   static map<char, int> charsMap;
   static vector<Lexeme> keywords;
   static vector<Lexeme> delimiters;

   vector<Lexeme> lexemes {};

   vector<string> identifierTable {};
   vector<string> constTable {};

   int idCounter;
   int constCounter;

   int line;
   int column;

   int pos;
   char currentChar;

   istream& infile;
   ostream& outfile;

   void lex_inp();
   void lex_space();
   void lex_digit();
   void lex_identifier();
   void lex_delimiter();

   void addError(const string& pt);

   int lookupIdentifier(const string& Ident) const;
   int lookupKeyword(const string& Ident) const;
   int lookupConstant(const string& Digit) const;
   int lookupDelimiter(char dm) const;

   Lexeme pushLexeme(int code, int line, int column, const string& name);

   void checkNewline();
   char readChar();
   bool hasSymbols() const;

   void init();
   static void initStaticFields();

   void printListing();
public:
   int errorsCount;

   Lexer(ostream& output, istream& input) : outfile(output), infile(input)
   {
      init();
   }

   bool analyze(bool print);
};