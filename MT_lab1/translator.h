#pragma once
#include "Const_Table.h"
#include "Variable_Table.h"

using namespace std;




class translator 
{

  
public:
   translator();
   bool LexAnalysis(string &codeFile, string &tokensFile, string &errorsFile);
   bool SyntacticAnalysis(string &tokensFile, string &errorsFile);
   void ReadParseTable(string &parseTableFile);
   void postfix_print(string file_tree);
   void semantic_analysis(string filename);

   class token
   {
   public:

      unsigned int tableNum;
      int chainNum;
      size_t num;
      
      token();
      token(unsigned int tableNum, int chainNum, size_t num);

      friend ofstream &operator <<(ofstream &os, token outpToken);
      friend ifstream &operator >>(ifstream &is, token inpToken);

   };
   struct TableParse
   {
      vector<string>terminal;
      int jump;
      int accept;
      int stack_;
      int return_;
      int error;
   };
   struct postfix_elem
   {
      string id;
      short int type;
      postfix_elem()
      {
         id = "", type = 0;
      }
      postfix_elem(string id_, int type_)
      {
         id = id_, type = type_;
      }
      postfix_elem(string id_)
      {
         id = id_, type = 1;
      }
      friend ostream &operator << (ostream &ostream_, const postfix_elem &pe_)
      {
         ostream_ << pe_.id;
         return ostream_;
      }

   };
private:
  
   bool priority_le(string what, string with_what);
   bool make_postfix(vector<token> t);
   vector<postfix_elem> postfix_record;
   vector<TableParse> tableParse;
   constTable<string>operators; //1
   constTable<string>keyWords;//2
   constTable<char>separators;//3
   constTable<char>figures;
   constTable<char>validChar;

   variableTable identifier;//4
   variableTable consts;//5

   ifstream streamCodeFile;
   ofstream streamTokenFile;
   ofstream streamErrorsFile;
   string getTokenText(token t);
   bool LineProcessing(string str);
   int CheckSym(char sym);
   static inline void trim(string &out_)
   {
      ltrim(out_);
      rtrim(out_);
   }
   static inline void ltrim(string &out_)
   {
      int notwhite = out_.find_first_not_of(" \t\n");
      out_.erase(0, notwhite);
   }
   static inline void rtrim(string &out_)
   {
      int notwhite = out_.find_last_not_of(" \t\n");
      out_.erase(notwhite + 1);
   }


};