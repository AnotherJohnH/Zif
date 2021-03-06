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
#include <vector>

#include "PLT/KeyCode.h"

#include "TRM/App.h"
#include "TRM/Curses.h"

#include "ConfigPage.h"
#include "GamePage.h"
#include "HomePage.h"
#include "InfoPage.h"
#include "ShellPage.h"
#include "RestorePage.h"

class Launcher : public TRM::App
{
protected:
   TRM::Device* term{nullptr};
   TRM::Curses  curses;

private:
   STB::Option<const char*> filename{'*', "*", "[<story-file>]", ""};
   std::vector<Page*>       page_stack;
   HomePage                 home_page;
   GamePage                 game_page;
   ConfigPage               config_page;
   InfoPage                 info_page;
   ShellPage                shell_page;
   RestorePage              restore_page;

   //! Check for a save file
   virtual bool hasSaveFile(const std::string& file) const = 0;

   //! Load and run a story file
   virtual int runGame(const char* file, bool restore) = 0;

   //! Open a new page, which is pushed onto the current page stack
   void openPage(Page& page)
   {
      page_stack.push_back(&page);
   }

   //! Close the current page, poping it of the page stack
   Page* closePage()
   {
      page_stack.pop_back();
      return page_stack.empty() ? nullptr : page_stack.back();
   }

   int doRunGame(const std::string& file, bool restore)
   {
      term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 1);
      int status = runGame(file.c_str(), restore);
      term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 0);
      curses.reset();
      return status;
   }

   int action(const std::string& cmd, const std::string& value = "")
   {
      int status = 0;

           if (cmd == "Quit")     { page_stack.clear(); }
      else if (cmd == "Games")    { openPage(game_page); }
      else if (cmd == "Settings") { openPage(config_page); }
      else if (cmd == "Info")     { openPage(info_page); }
      else if (cmd == "Shell")    { openPage(shell_page); }
      else if (cmd == "Select")
      {
         if (hasSaveFile(value))
         {
            restore_page.setFilename(value);
            openPage(restore_page);
         }
         else
         {
            status = doRunGame(value, /* restore */ false);
         }
      }
      else if (cmd == "Start")
      {
         status = doRunGame(value, /* restore */ false);
         closePage();
      }
      else if (cmd == "Resume")
      {
         status = doRunGame(value, /* restore */ true);
         closePage();
      }

      return status;
   }

   // Implement TRM::App

   virtual int startTerminalApp(TRM::Device& term_) override
   {
      term = &term_;
      curses.setDevice(&term_);

      config_page.setTermDevice(term_);

      term->ioctl(TRM::Device::IOCTL_TERM_CURSOR, 0);

      if (strcmp(filename, "") != 0)
      {
         return action("Select", filename.get());
      }

      openPage(home_page);

      std::string cmd, value;

      while(!page_stack.empty())
      {
         Page* page = page_stack.back();

         if (page->show(program))
         {
            if (page->back())
            {
               closePage();
            }
         }
         else
         {
            switch(curses.getch())
            {
            case -1:
               action("Quit");
               break;

            case PLT::UP:
               page->up();
               break;

            case PLT::DOWN:
               page->down();
               break;

            case PLT::PAGE_UP:
               page->pageUp();
               break;

            case PLT::PAGE_DOWN:
               page->pageDown();
               break;

            case PLT::HOME:
               page->home();
               break;

            case PLT::END:
               page->end();
               break;

            case ' ':
            case '\n':
            case PLT::SELECT:
            case PLT::RIGHT:
               if (page->select(cmd, value))
               {
                  action(cmd, value);
               }
               break;

            case PLT::BREAK:
               page->home();
               while(page_stack.size() > 1)
               {
                  if (page->back())
                  {
                     closePage();
                  }
               }
               break;

            case PLT::LEFT:
            case PLT::ESCAPE:
               if (page->back())
               {
                  closePage();
               }
               break;
            }
         }
      }

      return 0;
   }

public:
   Launcher(const char*  program,
            const char*  description,
            const char*  link,
            const char*  author,
            const char*  copyright_year,
            const char*  args_help)
      : App(program, description, link, author, copyright_year, args_help)
      , home_page(curses)
      , game_page(curses)
      , config_page(curses)
      , info_page(curses, description, link, author, copyright_year)
      , shell_page(curses, "zif")
      , restore_page(curses)
   {
   }
};

#endif // LAUNCHER_H
