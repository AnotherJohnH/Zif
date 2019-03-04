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

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <cctype>
#include <cstring>
#include <utility>

#include "PLT/File.h"
#include "PLT/KeyCode.h"

#include "TRM/App.h"
#include "TRM/Curses.h"

#include "HomePage.h"
#include "InfoPage.h"
#include "GamePage.h"
#include "ConfigPage.h"

class Launcher : public TRM::App
{
protected:
   TRM::Device* term{nullptr};
   TRM::Curses  curses;

private:
   const char* filename{nullptr};
   bool        quit{false};
   HomePage    home_page;
   GamePage    game_page;
   ConfigPage  config_page;
   InfoPage    info_page;
   Page*       active_page{nullptr};

   //!
   void drawHeader()
   {
      curses.clear();

      curses.attron(TRM::A_REVERSE);

      curses.move(1, 1);
      for(unsigned i = 0; i < curses.cols; ++i)
      {
         curses.addch(' ');
      }

      curses.attron(TRM::A_BOLD);
      curses.mvaddstr(1, 3, program);
      curses.attroff(TRM::A_BOLD);

      std::string title;
      active_page->title(title);
      curses.mvaddstr(1, 3 + strlen(program) + 2, title.c_str());

      char buffer[32]; // TODO stop using char[] and sprintf

      int32_t power = PLT::Info::get(PLT::Info::PWR_PERCENT);
      if (power > 0)
      {
         sprintf(buffer, " %02d%%", power);
         curses.mvaddstr(1, curses.cols - 11, buffer);
      }

      PLT::Rtc::DateAndTime date_and_time;
      if (PLT::Rtc::getDateAndTime(date_and_time))
      {
         sprintf(buffer, " %02u:%02u", date_and_time.hour, date_and_time.minute);
      }
      else
      {
         strcpy(buffer, " --:--");
      }
      curses.mvaddstr(1, curses.cols - 7, buffer);

      curses.attroff(TRM::A_REVERSE);
   }

   virtual void parseArg(const char* arg_) override { filename = arg_; }

   virtual int startTerminalApp(TRM::Device& term_) override
   {
      term = &term_;
      curses.setDevice(&term_);

      config_page.setTermDevice(term_);

      return filename ? runGame(filename)
                      : viewer();
   }

   //! Load and run a story file
   virtual int runGame(const char* file) = 0;

   void setPage(Page& page)
   {
      active_page = &page;
   }

   void action(const std::string& cmd, const std::string& value)
   {
      if (cmd == "Quit")
      {
         quit = true;
      }
      else if (cmd == "Games")
      {
         setPage(game_page);
      }
      else if (cmd == "Settings")
      {
         setPage(config_page);
      }
      else if (cmd == "Info")
      {
         setPage(info_page);
      }
      else if (cmd == "Run")
      {
         term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 1);

         runGame(value.c_str());

         term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 0);
      }
   }

   int viewer()
   {
      term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 0);

      setPage(home_page);

      std::string cmd, value;

      while(!quit)
      {
         drawHeader();

         active_page->show();

         switch(curses.getch())
         {
         case -1:
            quit = true;
            break;

         case PLT::UP:
            active_page->up();
            break;

         case PLT::DOWN:
            active_page->down();
            break;

         case ' ':
         case '\n':
         case PLT::SELECT:
         case PLT::RIGHT:
         case PLT::PAGE_DOWN:
            if (active_page->select(cmd, value))
            {
               action(cmd, value);
            }
            break;

         case PLT::ESCAPE:
         case PLT::PAGE_UP:
         case PLT::LEFT:
            if (active_page->back())
            {
               setPage(home_page);
            }
            break;
         }
      }

      curses.clear();

      return 0;
   }

public:
   Launcher(const char*  program,
            const char*  description,
            const char*  link,
            const char*  author,
            const char*  version,
            const char*  copyright_year,
            const char*  args_help,
            const char*  config_file)
      : App(program, description, link, author, version, copyright_year, args_help)
      , home_page(curses)
      , game_page(curses, config_file)
      , config_page(curses)
      , info_page(curses, program, description, link, author, version, copyright_year)
   {
   }
};

#endif // LAUNCHER_H
