//------------------------------------------------------------------------------
// Copyright (c) 2016-2019 John D. Haughton
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------

#ifndef SELECTOR_ITEM_H
#define SELECTOR_ITEM_H

#include <cctype>
#include <cstdlib>
#include <vector>

#include "Item.h"

class SelectorItem : public Item
{
public:
   SelectorItem(Owner*             owner,
                unsigned           row_,
                unsigned           col_,
                const std::string& text_,
                const std::string& choices_,
                const std::string& unit_ = "")
      : Item(owner, row_,col_, text_)
      , unit(unit_)
   {
      // First choice, no selection, is a aingle space
      choice_list.push_back(" ");

      std::string choice;
      for(const auto& ch : choices_)
      {
         if (ch == ',')
         {
            choice_list.push_back(choice);
            choice = "";
         }
         else
         {
            choice += ch;
         }
      }
      choice_list.push_back(choice);
   }

   const std::string& get() const
   {
      return choice_list.at(index);
   }

   signed getInt() const
   {
      return atoi(get().c_str());
   }

   void set(const std::string& value)
   {
      for(size_t i = 0; i < choice_list.size(); i++)
      {
         if (choice_list[i] == value)
         {
            index = i;
            break;
         }
      }
   }

   void setInt(signed value)
   {
      set(std::to_string(value));
   }

private:
   virtual void draw(TRM::Curses& curses, bool active) override
   {
      curses.attron(TRM::A_BOLD);
      Item::draw(curses, active);
      curses.attroff(TRM::A_BOLD);

      curses.addstr(" : ");

      if (active)
      {
         curses.attron(TRM::A_REVERSE);
      }

      curses.addstr(choice_list.at(index).c_str());

      if ((unit != "") && isdigit(choice_list[index][0]))
      {
         curses.addch(' ');
         curses.addstr(unit.c_str());
      }

      if (active)
      {
         curses.attroff(TRM::A_REVERSE);
      }
   }

   virtual bool select(std::string& cmd, std::string& value) override
   {
      index++;
      if (index == choice_list.size()) index = 1;

      cmd = text;
      value = choice_list.at(index);
      return true;
   }

   std::vector<std::string> choice_list;
   size_t                   index{0};
   std::string              unit;
};

#endif
