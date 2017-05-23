//------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
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

#ifndef ZLAUNCHER_H
#define ZLAUNCHER_H

#include <cstdio>
#include <cstring>

#include "PLT/Curses.h"
#include "PLT/KeyCode.h"

#include "ZifVersion.h"
#include "ZMachine.h"
#include "ZOptions.h"


//! ZMachine launcher
class ZLauncher : public PLT::Curses
{
private:
   PLT::Device&   term;
   const char*    config_file{nullptr};
   ZOptions       zoptions;
   unsigned       cursor{0};
   unsigned       cursor_limit{0};
   bool           quit{false};
   char           path[FILENAME_MAX] = {};
   char           selection[FILENAME_MAX] = {};
   bool           selection_is_dir{false};

   //! Get the next line with content from the given file stream
   static bool getLine(FILE* fp, char* file, size_t size)
   {
      while(fgets(file, size, fp))
      {
         if (file[0] != '#')
         {
            char* s = strchr(file, '\n');
            if (s) *s = '\0';
            return true;
         }
      }

      return false;
   }

   //! 
   void drawHeader()
   {
      clear();

      attron(PLT::A_REVERSE);
      move(1,1);
      clrtoeol();
      mvaddstr(1, 3, PROGRAM);
      attroff(PLT::A_REVERSE);

      attron(PLT::A_BOLD);
      mvaddstr(3, 3, path);
      attroff(PLT::A_BOLD);
   }

   //! 
   void drawList()
   {
      FILE* fp = fopen(config_file, "r");
      if (fp == nullptr)
      {
         return;
      }

      char prev[FILENAME_MAX];
      prev[0] = '\0';

      const unsigned first_row = 4;

      for(unsigned index = 0; (first_row + index)<lines; )
      {
         // Read one line from the config gile
         char line[FILENAME_MAX];
         if (!getLine(fp, line, sizeof(line)))
         {
            cursor_limit = index - 1;
            break;
         }

         // Does the start of the line match the current path
         if (strncmp(line, path, strlen(path)) == 0)
         {
            char* entry = line + strlen(path);

            // Extract entry and check if it is a directory
            bool is_dir = false;
            char* slash = strchr(entry, '/');
            if (slash != nullptr)
            {
               *slash = '\0';
               is_dir = true;
            }

            // Is this entry different to the previous entry
            if (strcmp(entry, prev) != 0)
            {
               strcpy(prev, entry);

               // Is this the currently selected entry
               if (index++ == cursor)
               {
                  strcpy(selection, entry);
                  selection_is_dir = is_dir;

                  attron(PLT::A_REVERSE);
               }

               if (entry[0] == '!')
               {
                  entry++;
               }

               mvaddstr(first_row + index, 3, entry);

               attroff(PLT::A_REVERSE);
            }
         }
      }

      fclose(fp);
   }

   void layoutText(unsigned l, unsigned c, const char* text)
   {
      move(l, c);

      const char* s = text;
      const char* o = s;

      for(unsigned x=3; true; x++)
      {
         if (isspace(*s) || (*s == '\0'))
         {
            if ((s - o) == 1)
            {
               addstr("\n\n");
               for(x = 1; x < c; ++x) addch(' ');
               o = s + 1;
            }
            else
            {
               if (x >= cols)
               {
                  addch('\n');
                  for(x = 1; x < c; ++x) addch(' ');
                  o++;
                  x += s - o;
               }

               for(; o != s; o++)
               {
                  char ch = *o;
                  if (ch == '\n') ch = ' ';
                  addch(ch);
               }
            }
         }

         if (*s++ == '\0') break;
      }
   }

   //! Display info page
   void doInfo()
   {
      drawHeader();

      mvaddstr( 3, 3, "Program     : "); addstr(PROGRAM);
      mvaddstr( 4, 3, "Description : "); addstr(DESCRIPTION);
      mvaddstr( 5, 3, "Author      : "); addstr(AUTHOR);
      mvaddstr( 6, 3, "Version     : "); addstr(VERSION);
      mvaddstr( 7, 3, "Built       : "); addstr(__TIME__); addstr(" "); addstr(__DATE__);
      mvaddstr( 8, 3, "Compiler    : "); layoutText(8, 17, __VERSION__);

      attron(PLT::A_BOLD);
      mvaddstr(10, 3, "Copyright (c) "); addstr(COPYRIGHT_YEAR); addstr(" "); addstr(AUTHOR);
      attroff(PLT::A_BOLD);

      layoutText(12, 3, LICENSE);

      (void) getch();
   }

   //! Implement an action
   void doAction(const char* cmd, const char* value = "")
   {
      if (strcmp(cmd, "Quit") == 0)
      {
         quit = true;
      }
      else if (strcmp(cmd, "Info") == 0)
      {
         doInfo();
      }
      else if (strcmp(cmd, "Video") == 0)
      {
#ifdef PROJ_TARGET_Kindle3
         const uint32_t dark  = 0x000000;
         const uint32_t light = 0xFFFFFF;
#else
         const uint32_t dark  = 0x382800;
         const uint32_t light = 0xF0F0E0;
#endif

         if (strcmp(value, "Inverse") == 0)
         {
            term.ioctl(PLT::Device::IOCTL_TERM_PALETTE, 0, light);
            term.ioctl(PLT::Device::IOCTL_TERM_PALETTE, 7, dark);
         }
         else if (strcmp(value,  "Normal") == 0)
         {
            term.ioctl(PLT::Device::IOCTL_TERM_PALETTE, 0, dark);
            term.ioctl(PLT::Device::IOCTL_TERM_PALETTE, 7, light);
         }
      }
      else if (strcmp(cmd, "Border") == 0)
      {
         term.ioctl(PLT::Device::IOCTL_TERM_BORDER, atoi(value));
         Curses::init();
      }
      else if (strcmp(cmd, "LineSpace") == 0)
      {
         term.ioctl(PLT::Device::IOCTL_TERM_LINE_SPACE, atoi(value));
         Curses::init();
      }
      else if (strcmp(cmd, "FontSize") == 0)
      {
         term.ioctl(PLT::Device::IOCTL_TERM_FONT_SIZE, atoi(value));
         Curses::init();
      }
   }


   //! Open a config directory (not a real directory)
   void openDir(const char* sub_directory)
   {
      strcat(path, sub_directory);
      strcat(path, "/");
      cursor = 0;
   }

   //! Close a config directory (not a real directory)
   void closeDir()
   {
      char* s = strrchr(path, '/');
      if (s)
      {
         *s = '\0';

         s = strrchr(path, '/');
         if (s)
         {
            s[1] = '\0';
         }
         else
         {
            path[0] = '\0';
         }

         cursor = 0;
      }
   }

   //! Select
   void doSelect(char* selection)
   {
      if (selection[0] == '!')
      {
         char* cmd = selection + 1;

         char* value = strchr(cmd, '=');
         if (value)
         {
            *value = '\0';

            doAction(cmd, value + 1);
         }
         else
         {
            doAction(cmd);
         }
      }
      else
      {
         // Load and run a game

         char story[FILENAME_MAX];

         strcpy(story, path);
         strcat(story, selection);

         run(story);
      }
   }

public:
   ZLauncher(PLT::Device& term_, const char* config_file_, ZOptions& zoptions_)
      : Curses(&term_)
      , term(term_)
      , config_file(config_file_)
      , zoptions(zoptions_)
   {
      doAction("Border",    "0");
      doAction("FontSize",  "18");
      doAction("LineSpace", "0");
      doAction("Video",     "Inverse");
   }

   //! Load and run a story file
   int run(const char* story)
   {
      ZMachine(&term, zoptions).open(story);
      return 0;
   }

   //! Enter the menu system
   int menu()
   {
      while(!quit)
      {
         drawHeader();
         drawList();

         int key = getch();

         switch(key)
         {
         case PLT::ESCAPE:
            doAction("Quit");
            break;

         case PLT::UP:
            cursor = cursor == 0 ? cursor_limit : cursor - 1;
            break;

         case PLT::DOWN:
            cursor = cursor == cursor_limit ? 0 : cursor + 1;
            break;

         case '\n':
         case PLT::SELECT:
         case PLT::RIGHT:
         case PLT::PAGE_DOWN:
            if (selection_is_dir)
            {
               openDir(selection);
            }
            else
            {
               doSelect(selection);
            }
            break;

         case PLT::PAGE_UP:
         case PLT::LEFT:
            closeDir();
            break;

         default:
            if (key < 0)
            {
               doAction("Quit");
            }
            break;
         }
      }

      clear();

      return 0;
   }
};

#endif
