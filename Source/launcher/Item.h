//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

      virtual void home()
      {
         active = getFirst();
      }

      virtual void end()
      {
         active = getLast();
      }

      virtual void pageUp()
      {
         active = getFirst();
      }

      virtual void pageDown()
      {
         active = getLast();
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

