//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

   virtual bool show(const std::string& program)
   {
      drawHeader(program);
      drawItems(curses);
      return false;
   }

protected:
   //!
   void drawHeader(const std::string& program)
   {
      curses.clear();

      curses.attron(TRM::A_REVERSE);

      curses.move(1, 1);
      for(unsigned i = 0; i < curses.cols; ++i)
      {
         curses.addch(' ');
      }

      curses.attron(TRM::A_BOLD);
      curses.mvaddstr(1, 3, program.c_str());
      curses.attroff(TRM::A_BOLD);

      title(title_text);
      curses.mvaddstr(1, 3 + program.size() + 2, title_text.c_str());

      // Battery power
      int32_t power = PLT::Info::get(PLT::Info::PWR_PERCENT);
      if (power >= 0)
      {
         power_text = std::to_string(power);
         power_text += '%';
      }
      curses.mvaddstr(1, curses.cols - 10, power_text.c_str());

      // Time
      PLT::Rtc::DateAndTime date_and_time;
      if (PLT::Rtc::getDateAndTime(date_and_time))
      {
         // There is a proper C++ way to do this I just don't like it
         time_text = "";
         if (date_and_time.hour < 10) time_text += '0';
         time_text += std::to_string(date_and_time.hour);
         time_text += ':';
         if (date_and_time.minute < 10) time_text += '0';
         time_text += std::to_string(date_and_time.minute);
      }
      curses.mvaddstr(1, curses.cols - 6, time_text.c_str());

      curses.attroff(TRM::A_REVERSE);
   }

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
   std::string  title_text{};
   std::string  power_text{};
   std::string  time_text{"--:--"};
};

