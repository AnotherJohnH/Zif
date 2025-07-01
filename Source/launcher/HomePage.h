//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "Page.h"
#include "ButtonItem.h"

//! Manage the home page
class HomePage : public Page
{
public:
   HomePage(TRM::Curses& curses_)
      : Page(curses_)
   {
   }

private:
   ButtonItem menu_games{    this, 4, 3, "Games"};
   ButtonItem menu_settings{ this, 5, 3, "Settings"};
   ButtonItem menu_info{     this, 6, 3, "Info"};
   ButtonItem menu_util{     this, 7, 3, "Shell"};
   ButtonItem menu_quit{     this, 8, 3, "Quit"};

   virtual bool back() override { return false; }
};

