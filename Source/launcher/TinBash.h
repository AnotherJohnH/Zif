//------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
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

#ifndef TIN_BASH_H
#define TIN_BASH_H

#include <string>
#include <vector>
#include <cstdio>
#include <unistd.h>

#include "TRM/Curses.h"

//! A simple shell utility
class TinBash
{
public:
   TinBash(TRM::Curses& curses_)
      : curses(curses_)
   {
   }

   //! Run the shells main execution loop
   void exec()
   {
      curses.clear();
      curses.curs_set(1);

      curses.addstr("TINBash (This Is Not Bash) -- extremely simple shell\n");
      curses.addstr("\n");

      quit = false;

      while(!quit)
      {
         read();
         split();
         run();
      }

      curses.curs_set(0);
   }

private:
   TRM::Curses&             curses;
   bool                     quit{false};
   std::string              cmd;
   std::string              ext_cmd;
   std::vector<std::string> argv;

   //! Read next user command
   void read()
   {
      cmd = "";

      curses.addstr("tin> ");

      bool end_of_line = false;
      while(!quit && !end_of_line)
      {
         int ch = curses.getch();
         if (ch < 0)
         {
            quit = true;
         }
         else
         {
            switch(ch)
            {
            case '\b':
               if (cmd != "")
               {
                  cmd.pop_back();
                  curses.addstr("\b \b");
               }
               break;

            case '\n':
               curses.addch('\n');
               end_of_line = true;
               break;

            case '\t':
               // TODO tab completion
               break;

            case PLT::BACK:
               quit = true;
               break;

            case PLT::UP: break;
            case PLT::DOWN: break;
            case PLT::LEFT: break;
            case PLT::RIGHT: break;

            case PLT::HOME:
               curses.clear();
               cmd = "";
               end_of_line = true;
               break;

            case PLT::MENU: break;
            case PLT::SELECT: break;

            default:
               if ((ch >= ' ') && (ch <= '~'))
               {
                  cmd += char(ch);
                  curses.addch(ch);
               }
               break;
            }
         }
      }
   }

   //! Split command into argument list
   void split()
   {
      argv.clear();

      bool in_space  = true;
      bool in_quotes = false;
      for(const auto& ch : cmd)
      {
         if (ch == '"')
         {
            if (!in_quotes)
            {
               argv.push_back("");
            }
            in_quotes = !in_quotes;
         }
         else if (in_quotes)
         {
            argv.back().push_back(ch);
         }
         else if ((ch == ' ') || (ch == '\t'))
         {
            in_space = true;
         }
         else
         {
            if (in_space)
            {
               in_space = false;
               argv.push_back("");
            }
            argv.back().push_back(ch);
         }
      }
   }

   //! Run command
   void run()
   {
      if (argv.size() > 0)
      {
         if (argv[0] == "exit")
         {
            quit = true;
         }
         else if (argv[0] == "restart")
         {
            doRestart();
         }
         else if (argv[0] == "cd")
         {
            if (chdir(argv[1].c_str()) < 0)
            {
               error("cd: No such file or directory");
            }
         }
         else if (argv[0] == "export")
         {
            // TODO env var setting
         }
         else
         {
            runExternal();
         }
      }
   }

   void doRestart()
   {
      static char* argv[2];
      argv[0] = (char*)"zif"; // naughty but everything is about to end
      argv[1] = nullptr;

      if (execve("./zif", argv, nullptr) < 0)
      {
         error("restart: Failed");
      }
   }

   //! Run command as an external process using the system shell
   void runExternal()
   {
      ext_cmd = cmd;
      ext_cmd += " 2>&1";

      FILE* pp = popen(ext_cmd.c_str(), "r");

      while(true)
      {
         int ch = fgetc(pp);
         if (ch < 0) break;
         curses.addch(ch);
      }

      pclose(pp);
   }

   void error(const std::string& message)
   {
       curses.addstr("tin: ");
       curses.addstr(message.c_str());
       curses.addch('\n');
   }
};

#endif
