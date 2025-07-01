//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

   const std::string        unit;
   std::vector<std::string> choice_list;
   size_t                   index{0};
};

