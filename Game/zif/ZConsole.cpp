//------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
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

#include <cstdio>

#include "ZConsole.h"

static FILE*  input_fp = nullptr;


ZConsole::ZConsole(PLT::Device* device_)
   : curses(device_)
{
   curses.raw();
   curses.noecho();

   input_fp = fopen("test.in", "r");
   if (input_fp)
   {
       // Fixed width
       curses.cols = 80;
   }
   else
   {
      input_fp = fopen("fast.in", "r");
   }
}


ZConsole::~ZConsole()
{
   if (input_fp) fclose(input_fp);
}


int ZConsole::getChar()
{
   if (input_fp != nullptr)
   {
      if (feof(input_fp))
      {
         input_fp = nullptr;
      }
      else
      {
         return fgetc(input_fp);
      }
   }

   return curses.getch();
}

