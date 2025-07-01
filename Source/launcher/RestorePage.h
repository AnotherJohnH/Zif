//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "Page.h"
#include "ButtonItem.h"

//! Manage the restor page
class RestorePage : public Page
{
public:
   RestorePage(TRM::Curses& curses_)
      : Page(curses_, "Restore Saved Game?")
   {
   }

   void setFilename(const std::string& filename_)
   {
      filename = filename_;
      active   = &yes;
   }

   virtual void title(std::string& text) override
   {
      text = filename;
   }

   virtual bool show(const std::string& program) override
   {
      Page::show(program);
      curses.mvaddstr(4, 3, "Restore saved game?");
      return false;
   }

   virtual bool select(std::string& cmd, std::string& value) override
   {
      cmd   = active == &yes ? "Resume" : "Start";
      value = filename;
      return true;
   }

private:
   ButtonItem  yes{this, 6, 5, "Yes"};
   ButtonItem  no{ this, 7, 5, "No"};
   std::string filename;
};

