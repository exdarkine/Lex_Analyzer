#include "Lexer.h"

#include <algorithm>
#include <iomanip>

#define DM_START 0
#define DIGIT_START 400
#define KEYWORD_START 700
#define ID_START 800

bool Lexer::initedStatic = false;
map<char, int> Lexer::charsMap {};
vector<Lexeme> Lexer::keywords {};
vector<Lexeme> Lexer::delimiters {};

void Lexer::initStaticFields()
{
   if(initedStatic) return;

   for (int i = 0; i <= 255; i++)
      charsMap[i] = CT_ERR;

   keywords.emplace_back("PROGRAM", KEYWORD_START);
   keywords.emplace_back("BEGIN", KEYWORD_START);
   keywords.emplace_back("END", KEYWORD_START);
   keywords.emplace_back("VA", KEYWORD_START);
   keywords.emplace_back("SIGNAL", KEYWORD_START);
   keywords.emplace_back("COMPLEX", KEYWORD_START);
   keywords.emplace_back("INTEGER", KEYWORD_START);
   keywords.emplace_back("FLOAT", KEYWORD_START);
   keywords.emplace_back("BLOCKFLOAT", KEYWORD_START);
   keywords.emplace_back("EXT", KEYWORD_START);

   delimiters.emplace_back(';');
   delimiters.emplace_back(',');
   delimiters.emplace_back(':');
   delimiters.emplace_back('(');
   delimiters.emplace_back(')');

   for(const Lexeme& dm : delimiters) {
      charsMap[dm.Code] = CT_DM;
   }

   for (char i = 'A'; i < 'Z'; i++)
      charsMap[i] = CT_LETTER;

   for (char i = '0'; i < '9'; i++)
      charsMap[i] = CT_DIGIT;

   for (char i = 8; i < 16; i++)
      charsMap[i] = CT_SPACE;
   charsMap[' '] = CT_SPACE;

   initedStatic = true;
}

void Lexer::init()
{
   initStaticFields();

   pos = 0;
   line = 1;
   column = 1;
   errorsCount = 0;
   lexemes.clear();

   identifierTable.clear();
   constTable.clear();

   constCounter = DIGIT_START;
   idCounter = ID_START;
}

Lexeme Lexer::pushLexeme(int code, int line, int column, const string& name)
{
   Lexeme buf(name, code, line, column);
   lexemes.push_back(buf);
   return buf;
}

bool Lexer::hasSymbols() const
{
   return !infile.eof();
}

bool Lexer::analyze(bool output)
{
   currentChar = (char)infile.get();

   if (hasSymbols())
      lex_inp();

   if (output)
      printListing();

   return errorsCount == 0;
}

void Lexer::checkNewline()
{
   if (currentChar == '\n')
   {
      line++;
      column = 0;
   }
}

void Lexer::lex_inp()
{
   while (hasSymbols())
   {
      switch (charsMap[currentChar])
      {
         case CT_SPACE:
            lex_space();
            break;
         case CT_LETTER:
            lex_identifier();
            break;
         case CT_DIGIT:
            lex_digit();
            break;
         case CT_DM:
            lex_delimiter();
            break;
         case CT_ERR:
            addError("Unexpected character met '" + string(1, (char) currentChar) + "'");
            readChar();
            break;
         case CT_EOF:
            return;
      }
   }
}

char Lexer::readChar()
{
   column++;
   pos++;
   currentChar = (char) infile.get();
   return currentChar;
}

void Lexer::lex_space()
{
   if (hasSymbols())
   {
      while ((hasSymbols()) && (charsMap[currentChar] == CT_SPACE))
      {
         checkNewline();
         readChar();
      }
      return;
   }
}

void Lexer::lex_digit()
{
   string buffer;
   while ((hasSymbols()) && (charsMap[currentChar] == CT_DIGIT))
   {
      buffer += currentChar;
      readChar();
   }
   if ((charsMap[currentChar] != CT_SPACE) &&
       (charsMap[currentChar] != CT_DM) &&
       (charsMap[currentChar] != CT_EOF))
   {
      addError("Delimiter or space expected");
      readChar();
   }

   int id = lookupConstant(buffer);
   if (id != -1)
   {
      pushLexeme(id + DIGIT_START, line, column - (int) buffer.length(), buffer);
   }
   else
   {
      pushLexeme(constCounter++, line, column - (int) buffer.length(), buffer);
      constTable.push_back(buffer);
   }

}

void Lexer::lex_identifier()
{
   string buffer;
   buffer += currentChar;
   readChar();
   while ((hasSymbols()) && (charsMap[currentChar] == CT_DIGIT || charsMap[currentChar] == CT_LETTER))
   {
      buffer += currentChar;
      readChar();
   }

   int id;
   if ((id = lookupKeyword(buffer)) != -1)
   {
      pushLexeme(id + KEYWORD_START, line, column - (int) buffer.length(), buffer);
   }
   else if ((id = lookupIdentifier(buffer)) != -1)
   {
      pushLexeme(id + ID_START, line, column - (int) buffer.length(), buffer);
   }
   else
   {
      id = idCounter++;
      pushLexeme(id, line, column - (int) buffer.length(), buffer);
      identifierTable.push_back(buffer);
   }
}

void Lexer::lex_delimiter()
{
   if (currentChar == '(')
   {
      readChar();
      if (currentChar == '*')
      {
         readChar();
         while (true)
         {
            while (currentChar != '*')
            {
               if (!hasSymbols())
               {
                  addError("End of comment expected");
                  return;
               }
               checkNewline();
               readChar();
            }
            readChar();
            if (currentChar == ')')
               break;
         }
         readChar();

      }
      else
      {
         Lexeme dm = delimiters[lookupDelimiter('(')];
         pushLexeme(dm.Code, line, (int) column - 1, dm.Name);
      }
   }
   else
   {
      Lexeme dm = delimiters[lookupDelimiter((char) currentChar)];
      pushLexeme(dm.Code, line, (int) column, dm.Name);
      readChar();
   }
}

void Lexer::addError(const string& pt)
{
   outfile << "Lexer error at " << line << ":" << column << " " << pt << endl;
   errorsCount++;
}

int Lexer::lookupIdentifier(const string& Ident) const
{
   for (int i = 0; i < identifierTable.size(); i++)
   {
      if (identifierTable[i] == Ident)
         return i;
   }
   return -1;
}

int Lexer::lookupKeyword(const string& Ident) const
{
   for (int i = 0; i < keywords.size(); i++)
   {
      if (keywords[i].Name == Ident)
         return i;
   }
   return -1;
}

int Lexer::lookupDelimiter(char dm) const
{
   for (int i = 0; i < keywords.size(); i++)
   {
      if (delimiters[i].Name[0] == dm)
         return i;
   }
   return -1;
}

int Lexer::lookupConstant(const string& Digit) const
{
   for (int i = 0; i < constTable.size(); i++)
   {
      if (constTable[i] == Digit)
         return i;
   }
   return -1;
}

void printLine(ostream& outfile, const string& code, const string& line, const string& col, const string& name)
{
   outfile << setw(10) << left <<  code
           << setw(10) << line
           << setw(10) << col
           << setw(30) << name << endl;
}

void Lexer::printListing()
{
   printLine(outfile, "Code", "Line", "Column", "Name");
   outfile << endl;
   for (auto& lexeme : lexemes)
      printLine(outfile, to_string(lexeme.Code), to_string(lexeme.Line), to_string(lexeme.Column), lexeme.Name);
}