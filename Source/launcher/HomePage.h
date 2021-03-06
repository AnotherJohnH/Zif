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

#ifndef HOME_PAGE_H
#define HOME_PAGE_H

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

#endif
