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
   unsigned cursor{0};
   unsigned cursor_limit{0};
   char     path[FILENAME_MAX]      = {};
   char     selection[FILENAME_MAX] = {};
   bool     selection_is_dir{false};

   virtual void title(std::string& text) override
   {
      text = path;
   }

   virtual void show(const std::string& program) override
   {
      drawHeader(program);

      selection_is_dir = false;

      const unsigned first_row = 3;

      FILE* fp = fopen(LIST_FILE, "r");
      if(fp == nullptr)
      {
         curses.mvaddstr(first_row, 3, "ERROR - failed to open \"" LIST_FILE "\"");
         return;
      }

      char prev[FILENAME_MAX];
      prev[0] = '\0';

      for(unsigned index = 0; (first_row + index) < curses.lines;)
      {
         // Read one line from the config gile
         char line[FILENAME_MAX];
         if(fgets(line, sizeof(line), fp) == nullptr)
         {
            cursor_limit = index - 1;
            break;
         }

         char* s = strchr(line, '\n');
         if (s != nullptr)
         {
            *s = '\0';
         }

         // Does the start of the line match the current path
         if(strncmp(line, path, strlen(path)) == 0)
         {
            char* entry = line + strlen(path);

            // Extract entry and check if it is a directory
            bool  is_dir = false;
            char* slash  = strchr(entry, '/');
            if(slash != nullptr)
            {
               *slash = '\0';
               is_dir = true;
            }

            // Is this entry different to the previous entry
            if(strcmp(entry, prev) != 0)
            {
               strcpy(prev, entry);

               // Is this the currently selected entry
               if(index++ == cursor)
               {
                  strcpy(selection, entry);
                  selection_is_dir = is_dir;

                  curses.attron(TRM::A_REVERSE);
               }

               curses.mvaddstr(first_row + index, 3, entry);

               curses.attroff(TRM::A_REVERSE);
            }
         }
      }

      fclose(fp);
   }

   virtual void up() override
   {
      cursor = cursor == 0 ? cursor_limit : cursor - 1;
   }

   virtual void down() override
   {
      cursor = cursor == cursor_limit ? 0 : cursor + 1;
   }

   virtual bool back() override
   {
      char* s = strrchr(path, '/');
      if(s == nullptr) return true;

      *s = '\0';

      s = strrchr(path, '/');
      if(s)
      {
         s[1] = '\0';
      }
      else
      {
         path[0] = '\0';
      }

      cursor = 0;
      return false;
   }

   virtual bool select(std::string& cmd, std::string& value) override
   {
      if(selection_is_dir)
      {
         strcat(path, selection);
         strcat(path, "/");
         cursor = 0;
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
