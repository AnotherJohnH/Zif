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

#ifndef RESTORE_PAGE_H
#define RESTORE_PAGE_H

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

   virtual void show(const std::string& program) override
   {
      Page::show(program);

      curses.mvaddstr(4, 3, "Restore saved game?");
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

#endif
