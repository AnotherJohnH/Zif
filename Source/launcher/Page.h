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

#ifndef PAGE_H
#define PAGE_H

#include "PLT/Info.h"
#include "PLT/Rtc.h"

#include "Item.h"

class Page : public Item::Owner
{
public:
   Page(TRM::Curses& curses_, const std::string& name_ = "")
      : curses(curses_)
      , name(name_)
   {
   }

   virtual void title(std::string& text)
   {
      text = name;
   }

   virtual void show()
   {
      drawItems(curses);
   }

protected:
   void layoutText(unsigned l, unsigned c, const char* text)
   {
      curses.move(l, c);

      const char* s = text;
      const char* o = s;

      for(unsigned x = 3; true; x++)
      {
         if(isspace(*s) || (*s == '\0'))
         {
            if((s - o) == 1)
            {
               curses.addstr("\n\n");
               for(x = 1; x < c; ++x)
                  curses.addch(' ');
               o = s + 1;
            }
            else
            {
               if(x >= curses.cols)
               {
                  curses.addch('\n');
                  for(x = 1; x < c; ++x)
                     curses.addch(' ');
                  o++;
                  x += s - o;
               }

               for(; o != s; o++)
               {
                  char ch           = *o;
                  if(ch == '\n') ch = ' ';
                  curses.addch(ch);
               }
            }
         }

         if(*s++ == '\0') break;
      }
   }

   TRM::Curses& curses;
   std::string  name;
};

#endif
