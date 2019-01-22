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

#include "ConsoleImpl.h"

#include "PLT/KeyCode.h"

static FILE* input_fp = nullptr;

bool ConsoleImpl::openInputFile(const char* filename)
{
   input_fp = fopen(filename, "r");
   return isInputFileOpen();
}

bool ConsoleImpl::isInputFileOpen() { return input_fp != nullptr; }

void ConsoleImpl::closeInputFile()
{
   fclose(input_fp);
   input_fp = nullptr;
}


int ConsoleImpl::getInput(unsigned timeout_ms)
{
   if(input_fp != nullptr)
   {
      if(feof(input_fp))
      {
         closeInputFile();
      }
      else
      {
         return fgetc(input_fp);
      }
   }

   curses.timeout(timeout_ms);

   int ch = curses.getch();

   // Some PLT::KeyCOde to ZSCII conversions
   switch(ch)
   {
   case PLT::UP:    return 0x81;
   case PLT::DOWN:  return 0x82;
   case PLT::LEFT:  return 0x83;
   case PLT::RIGHT: return 0x84;

   case PLT::F1:    return 0x85;
   case PLT::F2:    return 0x86;
   case PLT::F3:    return 0x87;
   case PLT::F4:    return 0x88;
   case PLT::F5:    return 0x89;
   case PLT::F6:    return 0x8A;
   case PLT::F7:    return 0x8B;
   case PLT::F8:    return 0x8C;
   case PLT::F9:    return 0x8D;
   case PLT::F10:   return 0x8E;
   case PLT::F11:   return 0x8F;
   case PLT::F12:   return 0x90;
   }

   return ch;
}
