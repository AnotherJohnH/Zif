//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "Page.h"
#include "TinBash.h"

//! Manage the shell page
class ShellPage : public Page
{
public:
   ShellPage(TRM::Curses& curses_, const std::string& program_)
      : Page(curses_, "TinBash")
      , tin(curses_, program_)
   {
   }

   //! Display info page
   virtual bool show(const std::string& program) override
   {
      tin.exec();
      return true;
   }

private:
   TinBash tin;
};

