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

#ifndef GAME_PAGE_H
#define GAME_PAGE_H

#include <cstdio>

#include "Page.h"

#define LIST_FILE "Games/list"

//! Manage the home page
class GamePage : public Page
{
public:
   GamePage(TRM::Curses& curses_)
      : Page(curses_)
   {
   }

private:
   std::string path;
   unsigned    cursor{0};
   unsigned    offset{0};
   unsigned    cursor_max{0};
   unsigned    offset_max{0};
   std::string selection;
   bool        selection_is_dir{false};

   std::string prev;

   virtual void title(std::string& text) override
   {
      text = path;
   }

   virtual void show(const std::string& program) override
   {
      drawHeader(program);

      const unsigned first_row = 4;

      cursor_max       = 0;
      offset_max       = 0;
      selection        = "";
      selection_is_dir = false;

      FILE* fp = fopen(LIST_FILE, "r");
      if(fp == nullptr)
      {
         curses.mvaddstr(first_row, 3, "ERROR - failed to open \"" LIST_FILE "\"");
         return;
      }

      prev = "";

      unsigned index = 0;
      unsigned pos   = 0;

      while(true)
      {
         // Read one line from the config gile
         char line[FILENAME_MAX];
         if(fgets(line, sizeof(line), fp) == nullptr)
         {
            break;
         }

         char* s = strchr(line, '\n');
         if (s != nullptr)
         {
            *s = '\0';
         }

         // Does the start of the line match the current path
         if(strncmp(line, path.c_str(), path.size()) == 0)
         {
            char* entry = line + path.size();

            // Extract entry and check if it is a directory
            bool  is_dir = false;
            char* slash  = strchr(entry, '/');
            if(slash != nullptr)
            {
               *slash = '\0';
               is_dir = true;
            }

            // Is this entry different to the previous entry
            if(prev != entry)
            {
               prev = entry;

               if (index++ >= offset)
               {
                  if ((first_row + pos) < (curses.lines - 1))
                  {
                     if(pos == cursor)
                     {
                        selection        = entry;
                        selection_is_dir = is_dir;

                        curses.attron(TRM::A_REVERSE);
                     }

                     curses.mvaddstr(first_row + pos, 3, entry);
                     curses.attroff(TRM::A_REVERSE);

                     pos++;
                  }
               }
            }
         }
      }

      cursor_max = pos - 1;
      offset_max = index - cursor_max - 2;

      fclose(fp);
   }

   virtual void up() override
   {
      if (cursor == 0)
      {
         if (offset > 0)
         {
            offset--;
         }
      }
      else
      {
         cursor--;
      }
   }

   virtual void down() override
   {
      if (cursor == cursor_max)
      {
         if (offset < offset_max)
         {
            offset++;
         }
      }
      else
      {
         cursor++;
      }
   }

   virtual bool back() override
   {
      size_t slash_pos = path.rfind('/');
      if (slash_pos == std::string::npos)
      {
         return true;
      }
      else
      {
         path.resize(slash_pos);

         size_t slash_pos = path.rfind('/');
         if (slash_pos == std::string::npos)
         {
            path = "";
         }
         else
         {
            path.resize(slash_pos + 1);
         }
      }

      cursor = offset = 0;
      return false;
   }

   virtual bool select(std::string& cmd, std::string& value) override
   {
      if(selection_is_dir)
      {
         path += selection;
         path += '/';
         cursor = offset = 0;
         return false;
      }
      else
      {
         cmd = "Run";
         value = "Games/";
         value += path;
         value += selection;
         return true;
      }
   }
};

#endif
