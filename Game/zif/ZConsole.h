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

#ifndef Z_CONSOLE_H
#define Z_CONSOLE_H

#include <cstdint>

#include "PLT/Curses.h"
#include "PLT/Device.h"

//! Console interface
class ZConsole
{
public:
   enum Attr
   {
      LINES,
      COLS,

      COLOURS,
      BOLD,
      ITALIC,

      FONT_HEIGHT,
      FONT_WIDTH,

      PICTURE_FONT,
      GRAPHIC_FONT,
      FIXED_FONT,

      READ_TIMEOUT
   };

   ZConsole(PLT::Device* dev_);

   ~ZConsole();

   //! Return console attribute
   unsigned getAttr(Attr attr) const
   {
      switch(attr)
      {
      case LINES:        return curses.lines;
      case COLS:         return curses.cols;

      case COLOURS:      return true;   // TODO platform specific
      case BOLD:         return true;
      case ITALIC:       return false;

      case FONT_HEIGHT:  return 10;     // TODO this value is fake
      case FONT_WIDTH:   return 6;      // TODO this value is fake

      case PICTURE_FONT: return false;
      case GRAPHIC_FONT: return false;
      case FIXED_FONT:   return true;

      case READ_TIMEOUT: return false;  // TODO

      default: return 0;
      }
   }

   void getCursorPos(unsigned& line, unsigned& col)
   {
      curses.getyx(line, col);
   }

   void clear()
   {
      curses.clear();
   }

   //! Select the current font
   void setFont(unsigned font_idx)
   {
      // TODO
   }

   //! Set (curses format) attributes
   void setAttributes(unsigned attr)
   {
      curses.attrset(attr);
   }

   //! Set (curses format) colours
   void setColours(unsigned fg_col, unsigned bg_col)
   {
      curses.colourset(fg_col, bg_col);
   }

   //! Move cursor
   void moveCursor(unsigned line, unsigned col)
   {
      curses.move(line, col);
   }

   //! Read ZSCII character
   bool read(uint16_t& zscii, unsigned timeout)
   {
      int ch;

      while(true)
      {
         // TODO timeout
         ch = getChar();
         if (ch < 0)
         {
            exit(0); // TODO this seems a bit severe!
         }
         else if (ch == 0x7F)
         {
            ch = '\b';
         }
         else if (ch < 0x7F)
         {
            break;
         }
      }

      zscii = ch;

      return true;
   }

   //! Write ZSCII character
   void write(uint16_t zscii)
   {
      curses.addch(zscii);
   }

   //! Report a message
   void message(const char* type, const char* message)
   {
      curses.clear();
      curses.attron(PLT::A_REVERSE);
      curses.move(1, 1);
      curses.clrtoeol();
      curses.mvaddstr(1, 1, "ZIF ");
      curses.addstr(type);
      curses.attroff(PLT::A_REVERSE);
      curses.mvaddstr(3, 1, message);
      (void) curses.getch();
   }

protected:
   PLT::Curses  curses;

private:
   int getChar();
};

#endif
