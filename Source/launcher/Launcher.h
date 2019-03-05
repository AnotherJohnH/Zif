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

#include <string>

#include "PLT/KeyCode.h"

#include "TRM/App.h"
#include "TRM/Curses.h"

#include "ConfigPage.h"
#include "GamePage.h"
#include "HomePage.h"
#include "InfoPage.h"
#include "RestorePage.h"

class Launcher : public TRM::App
{
protected:
   TRM::Device* term{nullptr};
   TRM::Curses  curses;

private:
   std::string  filename{};
   HomePage     home_page;
   GamePage     game_page;
   ConfigPage   config_page;
   InfoPage     info_page;
   RestorePage  restore_page;

   //! Check for a save file
   virtual bool hasSaveFile(const std::string& file) const = 0;

   //! Load and run a story file
   virtual int runGame(const char* file, bool restore) = 0;

   void action(Page*& page, const std::string& cmd, const std::string& value = "")
   {
           if (cmd == "Quit")     { page = nullptr; }
      else if (cmd == "Home")     { page = &home_page; }
      else if (cmd == "Games")    { page = &game_page; }
      else if (cmd == "Settings") { page = &config_page; }
      else if (cmd == "Info")     { page = &info_page; }
      else if (cmd == "Select")
      {
         if (hasSaveFile(value))
         {
            restore_page.setFilename(value);
            page = &restore_page;
         }
         else
         {
            action(page, "Start", value);
         }
      }
      else if (cmd == "Start")
      {
         term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 1);
         runGame(value.c_str(), /* restore */ false);
         term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 0);
         page = &game_page;
      }
      else if (cmd == "Resume")
      {
         term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 1);
         runGame(value.c_str(), /* restore */ true);
         term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 0);
         page = &game_page;
      }
   }

   // Implement TRM::App

   virtual void parseArg(const char* arg_) override
   {
      filename = arg_;
   }

   virtual int startTerminalApp(TRM::Device& term_) override
   {
      term = &term_;
      curses.setDevice(&term_);

      config_page.setTermDevice(term_);

      term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 0);

      Page* page = nullptr;

      if (filename == "")
      {
         action(page, "Home");
      }
      else
      {
         action(page, "Run", filename);
      }

      std::string cmd, value;

      while(page != nullptr)
      {
         page->show(program);

         switch(curses.getch())
         {
         case -1:
            action(page, "Quit");
            break;

         case PLT::UP:
            page->up();
            break;

         case PLT::DOWN:
            page->down();
            break;

         case ' ':
         case '\n':
         case PLT::SELECT:
         case PLT::RIGHT:
         case PLT::PAGE_DOWN:
            if (page->select(cmd, value))
            {
               action(page, cmd, value);
            }
            break;

         case PLT::ESCAPE:
         case PLT::PAGE_UP:
         case PLT::LEFT:
            if (page->back())
            {
               action(page, "Home");
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
            const char*  args_help)
      : App(program, description, link, author, version, copyright_year, args_help)
      , home_page(curses)
      , game_page(curses)
      , config_page(curses)
      , info_page(curses, description, link, author, version, copyright_year)
      , restore_page(curses)
   {
   }
};

#endif // LAUNCHER_H
