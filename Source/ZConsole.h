//------------------------------------------------------------------------------
// Copyright (c) 2016-2017 John D. Haughton
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
#include <cctype>

#include "PLT/Curses.h"
#include "PLT/Device.h"

#include "ZOptions.h"

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

   ZConsole(PLT::Device* device_)
      : curses(device_)
   {
      int status = device_->ioctl(PLT::Device::IOCTL_TERM_FONTS);
      if (status > 0)
      {
         num_fonts_avail = status;
      }

      status = device_->ioctl(PLT::Device::IOCTL_TERM_COLOURS);
      colours_avail = status > 1;
   }

   ~ZConsole()
   {
      if (isInputFileOpen())
      {
         closeInputFile();
      }
   }

   void init(ZOptions& options, uint8_t version)
   {
      if (options.input != nullptr)
      {
         openInputFile(options.input);
         if (!isInputFileOpen())
         {
            // error("Failed to open input file \"%s\"", options.input);
         }
      }

      if (options.width != 0)
      {
         curses.cols = options.width;
      }

      screen_enable = !options.batch;

      if (!screen_enable) return;

      curses.raw();
      curses.noecho();
      curses.clear();

      extended_colours = version == 6;
   }

   //! Return console attribute
   unsigned getAttr(Attr attr) const
   {
      switch(attr)
      {
      case LINES:        return curses.lines;
      case COLS:         return curses.cols;

      case COLOURS:      return colours_avail;
      case BOLD:         return true;
      case ITALIC:       return true;

      case FONT_HEIGHT:  return 1;
      case FONT_WIDTH:   return 1;

      case PICTURE_FONT: return false;
      case GRAPHIC_FONT: return false;
      case FIXED_FONT:   return true;

      case READ_TIMEOUT: return true;

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
   bool setFont(unsigned font_idx)
   {
      if (screen_enable && (font_idx <= num_fonts_avail))
      {
         curses.fontset(font_idx - 1);
         return true;
      }

      return font_idx == 1;
   }

   //! Set (curses format) attributes
   void setAttributes(unsigned attr)
   {
      if (!screen_enable) return;

      // The spec states that styles can be combined but is not required
      // the following allows combined styles
      if (attr == 0)
         curses.attrset(0);
      else
         curses.attron(attr);
   }

   //! Set colours
   void setColours(signed fg, signed bg)
   {
      if (!screen_enable) return;

      convertCodeToColour(fg_col, fg);

      if (fg_col >= COL_EXT_BASE)
         curses.extfgcolour(fg_col & 0xFF);
      else
         curses.fgcolour(fg_col);

      convertCodeToColour(bg_col, bg);

      if (bg_col >= COL_EXT_BASE)
         curses.extbgcolour(bg_col & 0xFF);
      else
         curses.bgcolour(bg_col);
   }

   //! Move cursor
   void moveCursor(unsigned line, unsigned col)
   {
      if (!screen_enable) return;

      curses.move(line, col);
   }

   //! Read ZSCII character
   //  Returns false on timeout
   bool read(uint16_t& zscii, unsigned timeout_100ms)
   {
      int ch;

      while(true)
      {
         ch = getInput(timeout_100ms);
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

      scroll = 0;
      only_white_space = true;

      return ch != 0;
   }

   //! Write ZSCII character
   void write(uint16_t zscii)
   {
      if (!screen_enable) return;

      if (zscii >= 128 )
      {
         curses.addch('?');
      }
      else
      {
         curses.addch(zscii);
      }

      if (!isspace(zscii))
      {
         only_white_space = false;
      }

      if ((zscii == '\n') && !isInputFileOpen())
      {
         scroll++;
         if (scroll == (curses.lines - 1))
         {
            waitForKey();
         }
      }
   }

   void waitForKey()
   {
      if (only_white_space) return;

      curses.addstr("...");
      curses.getch();
      curses.addstr("\b\b\b   \b\b\b");

      scroll = 0;
      only_white_space = true;
   }

protected:
   PLT::Curses  curses;

private:
   bool openInputFile(const char* filename_);
   void closeInputFile();
   bool isInputFileOpen();

   int  getInput(unsigned timeout_ms);

   // Convert Z colour codes to a curses colour index
   void convertCodeToColour(unsigned& current, signed code)
   {
      if (code == 0)
      {
         // no change
      }
      else if (code == 1)
      {
         current = COL_DEFAULT;
      }
      else if ((code >= 2) && (code <= 9))
      {
         // black, red, green, yellow, blue, magenta, cyan, white
         current = code - 2;
      }
      else if (extended_colours)
      {
         switch(code)
         {
         case -1: break; // TODO colour of pixel under cursor
         case 10: current = COL_EXT_BASE + 250; break; // ANSI 256-colour mode light grey
         case 11: current = COL_EXT_BASE + 244; break; // ANSI 256-colour mode medium grey
         case 12: current = COL_EXT_BASE + 237; break; // ANSI 256-colour mode dark grey
         default: break;
         }
      }
   }

   static const unsigned COL_NRM_BASE = 0;
   static const unsigned COL_DEFAULT  = 9;
   static const unsigned COL_EXT_BASE = 0x100;

   unsigned  num_fonts_avail{1};
   bool      colours_avail{true};
   unsigned  scroll{0};
   bool      only_white_space{true};
   bool      screen_enable{true};
   bool      extended_colours{false};
   unsigned  fg_col{COL_DEFAULT};
   unsigned  bg_col{COL_DEFAULT};
};

#endif