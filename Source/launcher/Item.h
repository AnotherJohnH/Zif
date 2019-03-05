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

#ifndef ITEM_H
#define ITEM_H

#include <string>

#include "TRM/Curses.h"

class Item
{
public:
   class Owner
   {
   public:
      Item* getFirst() { return first; }
      Item* getLast()  { return last;  }

      //! Maintain a double linked list of items
      void add(Item* item)
      {
         if (first == nullptr)
         {
            first = item;
         }
         else
         {
            first->prev = item;
            item->prev = last;
            item->next = first;
            last->next  = item;
         }
         last = item;
      }

      //! Move to the previous active item in the page
      virtual void up()
      {
         if (active != nullptr)
         {
            active = active->getPrev();
         }
      }

      //! Move to the next active item in the page
      virtual void down()
      {
         if (active != nullptr)
         {
            active = active->getNext();
         }
      }

      virtual bool back()
      {
         return true;
      }

      //! Select the active item
      virtual bool select(std::string& cmd, std::string& value)
      {
         if (active == nullptr) return false;

         return active->select(cmd, value);
      }

      //! Draw all the items
      void drawItems(TRM::Curses& curses)
      {
         if (getFirst() == nullptr) return;

         if (active == nullptr)
         {
            active = getFirst();
         }

         for(Item* item = getFirst(); true; item = item->getNext())
         {
            item->draw(curses, item == active);
            if (item == last) break;
         }
      }

   protected:
      Item* active{nullptr};

   private:
      Item* first{nullptr};
      Item* last{nullptr};
   };

   Item(Owner*              owner,
         unsigned           row_,
         unsigned           col_,
         const std::string& text_)
      : text(text_)
      , row(row_)
      , col(col_)
   {
      owner->add(this);
   }

   Item* getPrev() { return prev; }
   Item* getNext() { return next; }

   virtual void draw(TRM::Curses& curses, bool active)
   {
      curses.mvaddstr(row, col, text.c_str());
   }

   virtual bool select(std::string& cmd, std::string& value)
   {
      return false;
   }

protected:
   std::string text;

private:
   Item*    prev{nullptr};
   Item*    next{nullptr};
   unsigned row;
   unsigned col;
};

#endif
