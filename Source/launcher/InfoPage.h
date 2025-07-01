//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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
            const std::string& copyright_year_)
      : Page(curses_, "Info")
      , description(description_)
      , link(link_)
      , author(author_)
      , copyright_year(copyright_year_)
   {
   }

   //! Display info page
   virtual bool show(const std::string& program) override
   {
      drawHeader(program);

      curses.mvaddstr(3, 3, "Program     : "); curses.addstr(program.c_str());
      curses.mvaddstr(4, 3, "Description : "); curses.addstr(description.c_str());
      curses.mvaddstr(5, 3, "Author      : "); curses.addstr(author.c_str());
      curses.mvaddstr(6, 3, "Version     : "); curses.addstr(PLT_VERSION);
      curses.mvaddstr(7, 3, "Link        : "); if(link != "") curses.addstr(link.c_str());

      curses.mvaddstr(8, 3, "Commit      : "); curses.addstr(PLT_COMMIT);
      curses.mvaddstr(9, 3, "Built       : ");
      curses.addstr(__TIME__); curses.addstr(" "); curses.addstr(__DATE__);

      curses.mvaddstr(10, 3, "Compiler    : "); layoutText(10, 17, __VERSION__);

      curses.attron(TRM::A_BOLD);
      curses.mvaddstr(12, 3, "Copyright (c) ");
      curses.addstr(copyright_year.c_str());
      curses.addstr(" ");
      curses.addstr(author.c_str());

      curses.attroff(TRM::A_BOLD);

      layoutText(14, 3, MIT_LICENSE);

      return false;
   }

private:
   const std::string description;
   const std::string link;
   const std::string author;
   const std::string copyright_year;
};

