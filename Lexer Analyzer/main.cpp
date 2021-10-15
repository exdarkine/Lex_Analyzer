#include <string>
#include <iostream>

#include "Lexer.h"

int main(int argc, char** argv)
{
   if (argc != 2)
   {
      cout << "Error: No arguments passed" << endl;
      return 1;
   }

   string directory = string((const char*) argv[1]);
   string in_path = directory + "/test.sig";
   string out_path = directory + "/generated.txt";

   fstream in_file;
   fstream out_file;

   in_file.open(in_path, fstream::in);
   if (!in_file.is_open())
   {
      std::cout << "Error: failed to open input file '" << in_path << "'\n";
      return 1;
   }

   out_file.open(out_path, fstream::out);
   if (!out_file.is_open())
   {
      std::cout << "Error: Failed to open output file '" << out_path << "'\n";
      in_file.close();
      return 1;
   }

   Lexer lexer(out_file, in_file);
   if (!lexer.analyze(true))
      std::cout << "Error: lexer failed with " + to_string(lexer.errorsCount) + " error(s)";

   out_file.close();
   in_file.close();

   return 0;
}