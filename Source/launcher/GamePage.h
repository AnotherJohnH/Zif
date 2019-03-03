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

#include "Page.h"
#include "ButtonItem.h"

//! Manage the home page
class GamePage : public Page
{
public:
   GamePage(TRM::Curses& curses_, const char* config_file)
      : Page(curses_)
      , opt_config('c', "config", "Use alternate config file", config_file)
   {
   }

private:
   STB::Option<const char*> opt_config{'c', "config", "Use alternate config file"};
   unsigned                 cursor{0};
   unsigned                 cursor_limit{0};
   char                     path[FILENAME_MAX]      = {};
   char                     selection[FILENAME_MAX] = {};
   bool                     selection_is_dir{false};

   //! Get the next line with content from the given file stream
   static bool getLine(PLT::File& file, char* buffer, size_t size)
   {
      while(file.getLine(buffer, size))
      {
         if(buffer[0] != '#')
         {
            char* s = strchr(buffer, '\n');
            if(s) *s = '\0';
            return true;
         }
      }

      return false;
   }

   virtual void title(std::string& text) override
   {
      text = path;
   }

   virtual void show() override
   {
      selection_is_dir = false;
      strcpy(selection, "!Quit");

      const unsigned first_row = 3;

      PLT::File file(nullptr, opt_config);

      if(!file.openForRead())
      {
         curses.mvaddstr(first_row, 3, "ERROR - failed to open \"");
         curses.addstr(file.getFilename());
         curses.addstr("\"");
         return;
      }

      char prev[FILENAME_MAX];
      prev[0] = '\0';

      for(unsigned index = 0; (first_row + index) < curses.lines;)
      {
         // Read one line from the config gile
         char line[FILENAME_MAX];
         if(!getLine(file, line, sizeof(line)))
         {
            cursor_limit = index - 1;
            break;
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

               if(entry[0] == '!')
               {
                  entry++;
               }

               curses.mvaddstr(first_row + index, 3, entry);

               curses.attroff(TRM::A_REVERSE);
            }
         }
      }
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
