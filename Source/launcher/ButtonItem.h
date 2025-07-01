//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "Item.h"

class ButtonItem : public Item
{
public:
   ButtonItem(Owner* owner, unsigned row_, unsigned col_, const std::string& text_)
      : Item(owner, row_, col_, text_)
   {
   }

private:
   virtual void draw(TRM::Curses& curses, bool active) override
   {
      if (active)
      {
         curses.attron(TRM::A_REVERSE);
      }

      Item::draw(curses, active);

      if (active)
      {
         curses.attroff(TRM::A_REVERSE);
      }
   }

   virtual bool select(std::string& cmd, std::string& value) override
   {
      cmd   = text;
      value = "";
      return true;
   }
};

