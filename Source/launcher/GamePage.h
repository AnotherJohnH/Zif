//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

   virtual bool show(const std::string& program) override
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
         return false;
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
                  if ((first_row + pos) < curses.lines)
                  {
                     if(pos == cursor)
                     {
                        selection        = entry;
                        selection_is_dir = is_dir;

                        curses.attron(TRM::A_REVERSE);
                     }

                     if (is_dir) curses.attron(TRM::A_BOLD);
                     curses.mvaddstr(first_row + pos, 3, entry);
                     curses.attroff(TRM::A_BOLD);

                     curses.attroff(TRM::A_REVERSE);

                     pos++;
                  }
               }
            }
         }
      }

      cursor_max = pos - 1;
      offset_max = index - cursor_max - 1;

      fclose(fp);

      return false;
   }

   virtual void home() override
   {
      offset = 0;
      cursor = 0;
   }

   virtual void end() override
   {
      offset = offset_max;
      cursor = cursor_max;
   }

   virtual void pageUp() override
   {
      if (offset > cursor_max)
      {
         offset -= cursor_max;
      }
      else if (offset == 0)
      {
         cursor = 0;
      }
      else
      {
         offset = 0;
      }
   }

   virtual void pageDown() override
   {
      if (offset < offset_max)
      {
         offset += cursor_max;
         if (offset > offset_max)
         {
            offset = offset_max;
         }
      }
      else
      {
         cursor = cursor_max;
      }
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
         cmd = "Select";
         value = "Games/";
         value += path;
         value += selection;
         return true;
      }
   }
};

