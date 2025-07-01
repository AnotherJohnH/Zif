//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

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

   // Some PLT::KeyCode to ZSCII conversions
   switch(ch)
   {
   case PLT::DELETE:    return 0x08;
   case PLT::BACKSPACE: return 0x08;
   case PLT::TAB:       return 0x09;
   case PLT::RETURN:    return 0x0A;
   case PLT::ESCAPE:    return 0x1B;

   case PLT::UP:        return 0x81;
   case PLT::DOWN:      return 0x82;
   case PLT::LEFT:      return 0x83;
   case PLT::RIGHT:     return 0x84;

   case PLT::F1:        return 0x85;
   case PLT::F2:        return 0x86;
   case PLT::F3:        return 0x87;
   case PLT::F4:        return 0x88;
   case PLT::F5:        return 0x89;
   case PLT::F6:        return 0x8A;
   case PLT::F7:        return 0x8B;
   case PLT::F8:        return 0x8C;
   case PLT::F9:        return 0x8D;
   case PLT::F10:       return 0x8E;
   case PLT::F11:       return 0x8F;
   case PLT::F12:       return 0x90;

   // Ignore
   case PLT::SELECT:    return 1;
   case PLT::END:       return 1;
   case PLT::PAGE_UP:   return 1;
   case PLT::PAGE_DOWN: return 1;
   case PLT::VOL_UP:    return 1;
   case PLT::VOL_DOWN:  return 1;
   case PLT::MENU:      return 1;

   // Quit game
   case PLT::BREAK:     return -1;
   }

   return ch;
}
