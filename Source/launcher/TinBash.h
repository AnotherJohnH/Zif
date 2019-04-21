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

#include <cstdio>
#include <string>
#include <vector>
#include <cstdio>
#include <unistd.h>

#include "TRM/Curses.h"

//! A simple shell utility
class TinBash
{
public:
   TinBash(TRM::Curses& curses_, const std::string& program_)
      : curses(curses_)
      , program(program_)
   {
   }

   //! Run the shells main execution loop
   void exec(const std::string& script = "")
   {
      curses.raw();
      curses.noecho();
      curses.clear();
      curses.curs_set(1);

      if (script == "")
      {
         curses.addstr("TINBash (This Is Not Bash) -- extremely simple shell\n\n");
         interactive = true;
      }
      else
      {
         openScript(script);
         interactive = false;
      }

      quit = false;

      while(!quit)
      {
         prompt = true;

         read();
         split();
         run();
      }

      curses.curs_set(0);
   }

private:
   TRM::Curses&             curses;
   std::string              program;
   bool                     interactive{true};
   FILE*                    script_fp{nullptr};
   bool                     prompt{false};
   bool                     quit{false};
   std::string              prev_cmd;
   std::string              cmd;
   std::string              ext_cmd;
   std::vector<std::string> argv;

   bool openScript(const std::string& filename)
   {
      std::string filepath = "Scripts/";
      filepath += filename;
      filepath += ".tin";

      FILE* fp = fopen(filepath.c_str(), "r");
      if (fp == nullptr) return false;

      script_fp = fp;
      return true;
   }

   //! Read next char
   int getNextChar()
   {
      if (script_fp != nullptr)
      {
         int ch = fgetc(script_fp);
         if (ch != EOF)
         {
            return ch;
         }
         fclose(script_fp);
         script_fp = nullptr;

         if (!interactive)
         {
            return -1;
         }
      }

      if (prompt)
      {
         prompt = false;
         curses.addstr("tin> ");
      }

      return curses.getch();
   }

   //! Read next user command
   void read()
   {
      prev_cmd = cmd;
      cmd = "";

      bool end_of_line = false;
      bool in_comment  = false;
      while(!quit && !end_of_line)
      {
         int ch = getNextChar();
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
               if (script_fp == nullptr)
               {
                  curses.addch('\n');
               }
               end_of_line = true;
               break;

            case '\t':
               // TODO tab completion
               break;

            case PLT::BREAK:
            case PLT::ESCAPE:
               quit = true;
               break;

            case PLT::UP:
               while(cmd != "")
               {
                  cmd.pop_back();
                  curses.addstr("\b \b");
               }
               for(const auto& ch : prev_cmd)
               {
                  cmd += ch;
                  curses.addch(ch);
               }
               break;

            case PLT::MENU: break;
            case PLT::DOWN: break;
            case PLT::LEFT: break;
            case PLT::RIGHT: break;

            case PLT::HOME:
               curses.clear();
               cmd = "";
               end_of_line = true;
               break;

            case PLT::SELECT: break;

            default:
               if ((ch >= ' ') && (ch <= '~'))
               {
                  if (ch == '#')
                  {
                     in_comment = true;
                  }
                  else if (!in_comment)
                  {
                     cmd += char(ch);
                  }

                  if (script_fp == nullptr)
                  {
                     curses.addch(ch);
                  }
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
              if (argv[0] == "exit")    { cmd_exit(); }
         else if (argv[0] == "restart") { cmd_restart(); }
         else if (argv[0] == "cd")      { cmd_cd(); }
         else if (argv[0] == "export")
         {
            // TODO env var setting
         }
         else if (!openScript(argv[0]))
         {
            runExternal();
         }
      }
   }

   //! 
   void cmd_exit()
   {
      quit = true;
   }

   //! Change directory
   void cmd_cd()
   {
      if (argv.size() >= 2)
      {
         if (chdir(argv[1].c_str()) < 0)
         {
            error("cd: No such file or directory");
         }
      }
   }

   //! Restart the application
   void cmd_restart()
   {
      static char* args[2];
      args[0] = (char*) program.c_str(); // bit naughty but the world is just about to end
      args[1] = nullptr;

      if (execve(program.c_str(), args, nullptr) < 0)
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
       curses.addstr("tin: ERROR - ");
       curses.addstr(message.c_str());
       curses.addch('\n');
   }
};

#endif
