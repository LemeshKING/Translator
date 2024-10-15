#pragma once
#include<iostream>
#include<vector>
#include<string>

using namespace std;

struct lexem
{
   string name;
   bool type = false;
   bool value = false;
   int variableValue = 0;
   lexem(){}
   lexem(string name)
   {
      this->name = name;
   }
};
class variableTable
{
   size_t size = 50;
   vector<vector<lexem>>table;
   int getHash(const string &name)
   {
      int hash = 0;
      for (size_t i = 0; i < name.size(); i++)
         hash += name[i];
      return hash % size;
   }
   size_t getNumber(const string &name)
   {
      for (size_t i = 0, hash = getHash(name); i < table[hash].size(); i++)
         if (name == table[hash][i].name)
            return i;
      return -1;
   }
public:
   variableTable()
   {
      table.resize(size);
   }
   variableTable(const size_t &size)
   {
      this->size = size;
      table.resize(size);
   }
   ~variableTable()
   {
      for (size_t i = 0; i < size; i++)
         table[i].clear();
      table.clear();
   }
   bool searchHashAndNumber(const string &name,  int &hash, size_t &n)
   {
      hash = getHash(name);
      n = getNumber(name);
      if (n == -1)
         return false;
      return true;
   }
   bool addElem(const string &name)
   {
      if (getNumber(name) != -1)
         return false;
      int hash = getHash(name);
      table[hash].push_back(lexem(name));
      return true;
   }
   bool addType(const int &hash, const size_t &n)
   {
      if (n < 0 || n > table[hash].size())
         return false;
      table[hash][n].type = true;
      return true;
   }
   bool serchLexem(const int &hash,const size_t &n, lexem &lexem)
   {
      if (n < 0 || n > table[hash].size())
         return false;
      lexem = table[hash][n];
      return true;
   }
   bool serchLexem(const string &name, lexem &lexem)
   {
      int hash = getHash(name);
      size_t n = getNumber(name);
      return serchLexem(hash, n, lexem);
   }
   bool addValue(const int &hash, const size_t &n)
   {
      if (n < 0 || n > table[hash].size())
         return false;
      table[hash][n].value = true;
      return true;
   }
   bool addValue(const string &name)
   {
      size_t n = getNumber(name);
      int hash = getHash(name);
      return addValue(hash, n);
   }
   bool addType(const string &name)
   {
      size_t n = getNumber(name);
      int hash = getHash(name);
      return addType(hash, n);
   }
   int GetValue(const string &name)
   {
      lexem l;
      serchLexem(name, l);
      return l.variableValue;
   }
   bool SetValue(const string &name, int &value)
   {
      int hash = getHash(name);
      size_t n = getNumber(name);
      table[hash][n].variableValue = value;
      return true;
   }
};