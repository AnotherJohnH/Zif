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

#ifndef BUTTON_ITEM_H
#define BUTTON_ITEM_H

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

#endif
