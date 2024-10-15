#include"Const_Table.h"
#include"Variable_Table.h"
#include"translator.h"

int main()
{
   translator translator;
   string s1 = "code.txt",s2 = "tokens.txt", s3 = "errors.txt", s4 = "postfix.txt";
   string str = "parse.txt";
   string s5 = "syntax_error.txt";
   if (translator.LexAnalysis(s1, s2, s3))
   {
      translator.ReadParseTable(str);
      translator.SyntacticAnalysis(s2, s5);
      translator.postfix_print(s4);
      translator.semantic_analysis(s4);
   }
   return 0;
   

}
