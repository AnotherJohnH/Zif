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

#ifndef INFO_PAGE_H
#define INFO_PAGE_H

#include <string>

#include "Page.h"

//! Manage the information page
class InfoPage : public Page
{
public:
   InfoPage(TRM::Curses& curses_,
            const std::string& description_,
            const std::string& link_,
            const std::string& author_,
            const std::string& version_,
            const std::string& copyright_year_)
      : Page(curses_, "Info")
      , description(description_)
      , link(link_)
      , author(author_)
      , version(version_)
      , copyright_year(copyright_year_)
   {
   }

   //! Display info page
   virtual void show(const std::string& program) override
   {
      drawHeader(program);

      curses.mvaddstr(3, 3, "Program     : "); curses.addstr(program.c_str());
      curses.mvaddstr(4, 3, "Description : "); curses.addstr(description.c_str());
      curses.mvaddstr(5, 3, "Link        : "); if(link != "") curses.addstr(link.c_str());
      curses.mvaddstr(6, 3, "Author      : "); curses.addstr(author.c_str());
      curses.mvaddstr(7, 3, "Version     : "); curses.addstr(version.c_str());

      curses.mvaddstr(8, 3, "Built       : ");
      curses.addstr(__TIME__); curses.addstr(" "); curses.addstr(__DATE__);

      curses.mvaddstr(9, 3, "Compiler    : "); layoutText(9, 17, __VERSION__);

      curses.attron(TRM::A_BOLD);
      curses.mvaddstr(11, 3, "Copyright (c) ");
      curses.addstr(copyright_year.c_str());
      curses.addstr(" ");
      curses.addstr(author.c_str());

      curses.attroff(TRM::A_BOLD);

      layoutText(13, 3, MIT_LICENSE);
   }

private:
   const std::string description;
   const std::string link;
   const std::string author;
   const std::string version;
   const std::string copyright_year;
};

#endif
