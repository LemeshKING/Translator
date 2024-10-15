#pragma once
#include<iostream>
#include<fstream>
#include<set>
#include<string>

using namespace std;

template<typename type> 
class constTable
{
   set<type> table;
public:
   constTable() {}
   ~constTable()
   {
      table.clear();
   }
   void addElem(const type &elem)
   {
      table.insert(elem);
   }
   void input(string fileName)
   {
      ifstream file(fileName);
      type elem;
      while (!file.eof())
      {
         file >> elem;
         addElem(elem);
      }
      file.close();
   }
   bool searchElem(type &elem, const size_t &n)
   {
      if (n < 0 || n > table.size())
         return false;
      typename set<type>::iterator it = table.begin();
      for (unsigned int i = 0; i < n; i++, it++);
      elem = *it;
      return true;
   }
   bool searchNumber(const type &elem, size_t &n)
   {
      n = distance(table.begin(), table.find(elem));
      if (n != table.size())
         return true;
      return false;
   }
};
