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

#include "Console.h"

static FILE* input_fp = nullptr;

bool Console::openInputFile(const char* filename)
{
   input_fp = fopen(filename, "r");
   return isInputFileOpen();
}

bool Console::isInputFileOpen() { return input_fp != nullptr; }

void Console::closeInputFile()
{
   fclose(input_fp);
   input_fp = nullptr;
}


int Console::getInput(unsigned timeout_100ms)
{
   if(input_fp != nullptr)
   {
      if(feof(input_fp))
      {
         closeInputFile();
      }
      else
      {
         char ch;
         ch = fgetc(input_fp);
         return ch;
      }
   }

   curses.timeout(timeout_100ms * 100);

   return curses.getch();
}
