#include "translator.h"
#include <sstream>
#include<stack>

translator::translator()
{
   operators.input("operators.txt");
   keyWords.input("keywords.txt");
   separators.input("separators.txt");
   figures.input("figures.txt");
   validChar.input("validchar.txt");

}

bool translator::LexAnalysis(string &codeFile, string &tokensFile, string &errorsFile)
{
   streamCodeFile.open(codeFile);
   streamErrorsFile.open(errorsFile);
   streamTokenFile.open(tokensFile);
   string str;
   bool error = false;
   while (!streamCodeFile.eof() && !error)
   {
      streamCodeFile >> str;
      error = LineProcessing(str);
   }

   streamCodeFile.close();
   streamErrorsFile.close();
   streamTokenFile.close();

   return !error;
}

bool translator::SyntacticAnalysis(string &tokensFile, string &errorsFile)
{
   ifstream in(tokensFile, ios::in);
   ofstream error(errorsFile);
   error.clear();
   token currToken, nextToken;
   stack<int> parseStack;
   bool error_flag = false;
   int curr_row = 0;
   bool have_type = false; // Находимся ли мы в строке с объявлением типа
   bool need_postfix = false;      // Нужно ли выполнять построение постфиксной записи для данной строки
   vector<token> code_expr_infix;  // Если да, то сюда помещаем токены в инфиксном (обычном) порядке
   bool eof_flag = in.eof();    // Флаг конца файла (чтобы считать последний токен)
   in >> currToken.tableNum >> currToken.chainNum >> currToken.num;
   in >> nextToken.tableNum >> nextToken.chainNum >> nextToken.num;
   while (!eof_flag && !error_flag)
   {
      string tokenStr = getTokenText(currToken);

      trim(tokenStr);
      if (currToken.tableNum == 4) tokenStr = "var_name";
      if (currToken.tableNum == 5) tokenStr = "const";
      bool find_terminal = false;
      //cout << "Curr Row = " << curr_row << endl;
      //cout << "Token: " << currToken.tableNum << " " << currToken.chainNum << " " << currToken.num << endl;
      //cout << "Token String: " << tokenStr << endl;
      for (int i = 0; i < (int)tableParse[curr_row].terminal.size() && !find_terminal; i++)
      {
         //cout << "Scan " << tableParse[curr_row].terminal[i] << " : ";
         if (tableParse[curr_row].terminal[i] == tokenStr)
            find_terminal = true;
         //cout << find_terminal << endl;
      }
      if (find_terminal)
      {
         if (tableParse[curr_row].stack_)
            parseStack.push(curr_row + 1);
         if (tableParse[curr_row].accept)
         {
            string oper = "";
            operators.searchElem(oper, nextToken.num);
            if (((tokenStr == "var_name" || tokenStr == "const") && (getTokenText(nextToken) == oper)) || tokenStr == "-")
               need_postfix = true;
            if (!have_type && tokenStr == "var_name")
            {
               lexem lex_var;
               identifier.serchLexem(currToken.chainNum, currToken.num, lex_var);
               if (lex_var.type == 0)
               {
                  error_flag = true;
                  error << "Syntax Error: Undefined identifier \"" << lex_var.name << "\"" << endl;
               }
               
            }
            if (tokenStr == "var_name" && getTokenText(nextToken) != "=" && !have_type )
            {
               lexem lex_var;
               identifier.serchLexem(currToken.chainNum, currToken.num, lex_var);
               if (lex_var.value == 0)
               {
                  error_flag = true;
                  error << "Syntax Error: Used variable without initialization.  \"" << lex_var.name << "\"" << endl;
               }
            }
            if (tokenStr == "var_name" && getTokenText(nextToken) == "=" && !have_type)
               identifier.addValue(getTokenText(currToken));
            bool flag_unary_minus = false;
            if ((curr_row == 64 || curr_row == 68) && need_postfix)
            {
               int one_hash;
               size_t n;
               consts.addElem("-1");
               consts.searchHashAndNumber("-1", one_hash, n);
               code_expr_infix.push_back(token(5, one_hash, n));
               size_t mult_pos;
               operators.searchNumber("*", mult_pos);
               code_expr_infix.push_back(token(1, -1, mult_pos));
               flag_unary_minus = true;
            }
            if (need_postfix && !flag_unary_minus)
               code_expr_infix.push_back(currToken);
            if (tokenStr == ";" || tokenStr == ",")
            {
               // Добавим все, что разобрали, в постфиксную запись
               if (!make_postfix(code_expr_infix))
                  error_flag = true;
              
              // Сбрасываем все флаги
               code_expr_infix.clear();

               need_postfix = false;

            }

            // Если закончили разбор объявления, сбросим флаг объявления
            if (tokenStr == ";")
               have_type = false;
            if (tokenStr == "int")
            {
               have_type = true;
            }
            if (tokenStr == "var_name" && have_type && curr_row == 88)
               identifier.addType(getTokenText(currToken));
            if (tokenStr == "var_name" && have_type && getTokenText(nextToken) == "=")
               identifier.addValue(getTokenText(currToken));
            eof_flag = in.eof();
            currToken = nextToken;
            if (!eof_flag)
               in >> nextToken.tableNum >> nextToken.chainNum >> nextToken.num;
         }
         if (tableParse[curr_row].return_)
         {
            if (!parseStack.empty())
            {
               curr_row = parseStack.top();
               parseStack.pop();
            }
            else // Если внезапно стек пуст
            {
               error_flag = true;

               error << "Syntax Error: Parse stack is empty!" << endl;
               error << "Return requested by row " << curr_row << " at token " << currToken.tableNum << " " << currToken.chainNum << " " << currToken.num
                  << " (value = \"" << getTokenText(currToken) << "\")" << endl;
            }
         }
         else
            curr_row = tableParse[curr_row].jump;
      }
      else
      {
         if (tableParse[curr_row].error)
         {
            error_flag = true;
            error << "Syntax Error: Unexpected terminal \"" << getTokenText(currToken) << "\"" << endl;
            error << "Must be: ";
            for (int i = 0; i < (int)tableParse[curr_row].terminal.size(); i++)
               error << "\"" << tableParse[curr_row].terminal[i] << "\" ";
            error << endl;
         }
         else
            curr_row++;
      }

   };
   if (!error_flag && !parseStack.empty())
   {
      error_flag = true;
      error << "Syntax Error: Parse stack isn`t empty!" << endl;
      error << "Size = " << parseStack.size() << endl;
      error << "Contains: ";
      while (!parseStack.empty())
      {
         error << "\"" << parseStack.top() << "\" " << endl;
         parseStack.pop();
      }
      error << endl;
   }

   in.close();
   error.close();
   return !error_flag;
}


   



void translator::ReadParseTable(string &parseTableFile)
{
   ifstream file(parseTableFile);
   string str;
   getline(file, str);
   while(!file.eof())
   {
      TableParse tmp;
      file >> str;
      stringstream a;
      str = "";
      while (str.length() == 0 || str.find("\t") != string::npos)
         getline(file, str, '\t');
      a.str(str);
      while(a.good())
      {
         a >> str;
         tmp.terminal.push_back(str);
      }
      file >> tmp.jump >> tmp.accept >> tmp.stack_ >> tmp.return_ >> tmp.error;
      tableParse.push_back(tmp);
   }
   file.close();

}

bool translator::LineProcessing(string str)
{
   if (str.size() != 0)
   {
      bool error2 = false;
      bool error = false;
      int firstSym = CheckSym(str[0]);
      size_t n = 0;
      int chain = 0;
      string contStr;
      int strEr = 0;
      size_t countPoints = 0;
      int i;
      string comment;
      bool commendEnd = false;
      switch (firstSym)
      {
      case -1:
         streamErrorsFile << "Ошибка в " << str << " Недопустимый символ." << endl;
         error = true;
         break;
      case 0:
         for (i = 1; i < str.size() && !error; i++)
            error = !(validChar.searchNumber(str[i], n) || figures.searchNumber(str[i], n));
         strEr = i;
         if (error)
            strEr--;
         contStr = str.substr(strEr);
         str = str.substr(0, strEr);
         if (keyWords.searchNumber(str, n))
            streamTokenFile << token(2, -1, n);
         else
         {
            identifier.addElem(str);
            identifier.searchHashAndNumber(str, chain, n);
            streamTokenFile << token(4, chain, n);
         }
         error = LineProcessing(contStr);
         break;
      case 1:
         
         for (i = 1; i < str.size() && !error && !error2; i++)
         {
            error = !(figures.searchNumber(str[i], n) || str[i] == '.');
            error2 = validChar.searchNumber(str[i], n);
         }
         if (error2)
         {
            streamErrorsFile << "Ошибка в " << str << " Недопустимый символ." << endl;
            break;
         }
         strEr = i;
         if (error)
            strEr--;
         error = false;
         contStr = str.substr(strEr);
         str = str.substr(0, strEr);
         if (!error)
         {
            consts.addElem(str);
            consts.searchHashAndNumber(str, chain, n);
            streamTokenFile << token(5, chain, n);
            error = LineProcessing(contStr);
         }
         break;
      case 2:
         if (str[0] != '/')
         {
            if (str.size() > 1)
            {
               contStr = str.substr(2);
               str = str.substr(0, 2);
            }
            error = !operators.searchNumber(str, n);
            if (error)
            {
               contStr = str.substr(1) + contStr;
               str = str.substr(0, 1);
               operators.searchNumber(str, n);
            }
            streamTokenFile << token(1, -1, n);
            error = LineProcessing(contStr);
         }
         else
         {
            if(str.size() > 1)
               switch (str[1])
               {
               case '*':
                  while (!commendEnd && !streamCodeFile.eof())
                  {
                     streamCodeFile >> comment;
                     i = comment.find("*/");
                     if (i != -1)
                        commendEnd = true;
                  }
                  if (!streamCodeFile.eof())
                  {
                     i = comment.find_first_of("*/");
                     contStr = comment.substr(i + 2);
                     error = LineProcessing(contStr);
                  }
                  else 
                  {
                     error = true;
                     streamErrorsFile << "Незакрытый комментарий" << endl;
                  }
                  break;
               case '/':
                  getline(streamCodeFile, comment);
                  break;
               default:
                  contStr = str.substr(1);
                  str = str.substr(0, 1);
                  operators.searchNumber(str, n);
                  streamTokenFile << (1, -1, n);
                  error = LineProcessing(contStr);
                  break;
               }
            else
            {
               operators.searchNumber(str, n);
               streamTokenFile << token(1, -1, n);
            }
         }
         break;
      case 3:
         contStr = str.substr(1);
         separators.searchNumber(str[0], n);
         streamTokenFile << token(3, -1, n);
         error = LineProcessing(contStr);
         break;
      default:
         streamErrorsFile << "Не получилось определить принадлежность первого символа" << endl;
         error = true;
         break;
      }
      if (error)
         return true;
      
   }
   return false;
}

int translator::CheckSym(char sym)
{
   string str(1,sym);
   size_t n;
   if(validChar.searchNumber(sym,n))
      return 0;
   if (figures.searchNumber(sym, n))
      return 1;
   if (operators.searchNumber(str, n))
      return 2;
   if (separators.searchNumber(sym, n))
      return 3;
   return -1;

}

bool translator::make_postfix(vector<token> t)
{
   stack<string> stack_temp;
   bool error_flag = false;
   int index = 0;
   while (index < (int)t.size() && !error_flag)
   {
      int i;
      for (i = index; i < (int)t.size() && !error_flag && getTokenText(t[i]) != ";" && getTokenText(t[i]) != ","; i++)
      {
         string token_text = getTokenText(t[i]);
         if (t[i].tableNum == 4 || t[i].tableNum == 5)
         {
            postfix_record.push_back(postfix_elem(token_text));
         }
         else if (token_text == "(")
         {
            stack_temp.push(token_text);
         }
         else if (token_text == ")")
         {
            while (!stack_temp.empty() && stack_temp.top() != "(")
            {
               string tmpstr = stack_temp.top();
               postfix_record.push_back(postfix_elem(tmpstr));
               stack_temp.pop();
            }
            if (stack_temp.empty())
            {
              
               cout << "Syntax Error: Unexpected \")\" !" << endl;
               error_flag = true;
            }
            else
            {
               stack_temp.pop();
            }
         }
         
         else if (t[i].tableNum == 1)
         {
            while (!stack_temp.empty() && priority_le(token_text, stack_temp.top()))
            {
               string tmpstr = stack_temp.top();
               postfix_record.push_back(postfix_elem(tmpstr));
               stack_temp.pop();
            }
            stack_temp.push(token_text);
         }
      }
      if (error_flag)
      {
         postfix_record.clear();
         return false;
      }
      else
      {
         while (!stack_temp.empty() && stack_temp.top() != "(" && stack_temp.top() != ")")
         {
            string tmpstr = stack_temp.top();
            postfix_record.push_back(postfix_elem(tmpstr, 1));
            stack_temp.pop();
         }
         if (!stack_temp.empty())
         {
            
            cout << "Syntax Error: Brackets balance error!" << endl;
            error_flag = true;
         }
      }
      if (error_flag)
      {
         postfix_record.clear();
         return false;
      }
      index = i + 1;
      postfix_record.push_back(postfix_elem(";", 3));
   }
   return true;

}

bool translator::priority_le(string what, string with_what)
{
   int pw = 0, pww = 0;
   if (what == "=" || what == "+=" || what == "-=" || what == "*=") pw = 1;
   else if (what == "!=" || what == ">" || what == "<" || what == "==") pw = 2;
   else if (what == "+" || what == "-") pw = 3;
   else pw = 4;
   if (with_what == "=" || with_what == "+=" || with_what == "-=" || with_what == "*=") pww = 1;
   else if (with_what == "!=" || with_what == ">" || with_what == "<" || with_what == "==") pww = 2;
   else if (with_what == "+" || with_what == "-") pww = 3;
   else if (with_what == "*") pww = 4;
   if (pw <= pww) return true;
   return false;

}

void translator::postfix_print(string file_tree)
{
   ofstream out(file_tree.c_str());
   cout << "Postfix notation:" << endl;
   for (int i = 0; i < (int)postfix_record.size(); i++)
   {
      cout << postfix_record[i] << " ";
      out << postfix_record[i] << " ";
      if (postfix_record[i].id == ";")
         out << "\n";
   }
   cout << endl;
   out.close();

}

void translator::semantic_analysis(string filename)
{
   setlocale(LC_ALL, "");
   stack<int> stk;
   string postfix;
   int op1, op2;
   stack<lexem> l;

   string ch = "";

   ifstream file("postfix.txt");
   while (!file.eof())
   {
      getline(file, postfix);
      stringstream x;
      x << postfix;
      while (x >> ch)
      {
         bool flag = true;
         for (int j = 0; j < ch.size() && flag; j++)
            if (!isdigit(ch[j]))
               flag = false;
         if (flag)
         {
            int digit = std::stoi(ch);
            stk.push(digit);
         }
         else if (ch == "+" || ch == "-" || ch == "*" || ch == "/")
         {
            char oper = ch[0];
            op2 = stk.top();
            stk.pop();
            op1 = stk.top();
            stk.pop();
            switch (oper) {
            case '+': stk.push(op1 + op2); break;
            case '-': stk.push(op1 - op2); break;
            case '*': stk.push(op1 * op2); break;
            case '/': stk.push(op1 / op2); break;
            }
         }
         else if (ch == "=" || ch == "+=" || ch == "-=" || ch == "*=" || ch =="/=")
         {
            switch (ch[0])
            {
            case '+': 
            {
               int k = l.top().variableValue + stk.top();
               stk.pop();
               stk.push(k);
               identifier.SetValue(l.top().name, k);
               break; 
            }
            case '-': 
            {
               int k = l.top().variableValue - stk.top();
               stk.pop();
               stk.push(k);
               identifier.SetValue(l.top().name, k);
               break; 
            }
            case '*': 
            { 
               int k = l.top().variableValue * stk.top();
               stk.pop();
               stk.push(k);
               identifier.SetValue(l.top().name, k); 
               break; 
            }
            case '/': 
            {
               int k = l.top().variableValue / stk.top();
               stk.pop();
               stk.push(k);
               identifier.SetValue(l.top().name, k); 
               break; 
            }
            case '=': identifier.SetValue(l.top().name, stk.top()); break;
               
            }
            l.pop();
         }
         else if(ch != ";")
         {
            lexem lex;
            identifier.serchLexem(ch, lex);
            if (l.empty())
               l.push(lex);
            else
               stk.push(lex.variableValue);

         }
      }
      if (!stk.empty())
      {
         cout << "Результат: " << stk.top() << endl;
         stk.pop();
      }
   }
}

string translator::getTokenText(token t)
{
   string str = "";
   char sym = '\0';
   lexem l("");
   switch (t.tableNum)
   {
   case 1:
      operators.searchElem(str, t.num);
      return str;
   case 2:
      keyWords.searchElem(str, t.num);
      return str;
   case 3:
      separators.searchElem(sym, t.num);
      str.append(&sym, 1);
      return str;
   case 4:
      identifier.serchLexem(t.chainNum,t.num,l);
      return l.name;
   case 5:
      consts.serchLexem(t.chainNum, t.num, l);
      return l.name;
   }
   return str;

}

translator::token::token()
{
   this->tableNum = 0;
   this->chainNum = 0;
   this->num = 0;
}




translator::token::token(unsigned int tableNum,  int chainNum, size_t num)
{
   this->tableNum = tableNum;
   this->chainNum = chainNum;
   this->num = num;
}

ofstream &operator<<(ofstream &os, translator::token outpToken)
{
   os << outpToken.tableNum << " " << outpToken.chainNum << " " << outpToken.num << endl;
   return os;
}

ifstream &operator>>(ifstream &is, translator::token inpToken)
{
   is >> inpToken.tableNum >> inpToken.chainNum >> inpToken.num;
   return is;
}
