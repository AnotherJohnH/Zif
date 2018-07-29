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

#ifndef CONSOLE_IMPL_H
#define CONSOLE_IMPL_H

#include <cctype>
#include <cstdint>

#include "TRM/Curses.h"
#include "TRM/Device.h"

#include "Console.h"
#include "Options.h"

//! Console implementation using the Platform TRM classes
class ConsoleImpl : public Console
{
public:
   ConsoleImpl(TRM::Device* device_, Options& options)
      : curses(device_)
   {
      int status = device_->ioctl(TRM::Device::IOCTL_TERM_FONTS);
      if(status > 0)
      {
         num_fonts_avail = status;
      }

      status        = device_->ioctl(TRM::Device::IOCTL_TERM_COLOURS);
      colours_avail = status > 1;

      if(options.input != nullptr)
      {
         openInputFile(options.input);
         if(!isInputFileOpen())
         {
            // error("Failed to open input file \"%s\"", options.input);
         }
      }

      if(options.width != 0)
      {
         curses.cols = options.width;
      }

      screen_enable = !options.batch;

      if(!screen_enable) return;

      curses.raw();
      curses.noecho();
      curses.clear();
   }

   ~ConsoleImpl()
   {
      if(isInputFileOpen())
      {
         closeInputFile();
      }
   }

   //! Return console attribute
   virtual unsigned getAttr(Attr attr) const override
   {
      // clang-format off
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
      // clang-format on
   }

   //! Get current position of cursor
   virtual void getCursorPos(unsigned& line, unsigned& col) override
   {
       curses.getyx(line, col);
   }

   //! Clear the console
   virtual void clear() override
   {
      curses.clear();
   }

   //! Select the current font
   virtual bool setFont(unsigned font_idx) override
   {
      if(screen_enable && (font_idx <= num_fonts_avail))
      {
         curses.fontset(font_idx - 1);
         return true;
      }

      return font_idx == 1;
   }

   //! Change the font style
   virtual void setFontStyle(FontStyle style_bit_mask) override
   {
      if(!screen_enable) return;

      // Convert ConsoleIf font style to curses attributes. Might be a 1-1
      // mapping of bits, but copying each bit is more robust
      unsigned curses_attr = 0;
      if(style_bit_mask & FONT_STYLE_REVERSE) curses_attr |= TRM::A_REVERSE;
      if(style_bit_mask & FONT_STYLE_BOLD)    curses_attr |= TRM::A_BOLD;
      if(style_bit_mask & FONT_STYLE_ITALIC)  curses_attr |= TRM::A_ITALIC;
      if(style_bit_mask & FONT_STYLE_FIXED)   curses_attr |= TRM::A_FIXED;

      curses.attrset(curses_attr);
   }

   //! Set foreground colour
   virtual void setForegroundColour(Colour colour) override
   {
      if(!screen_enable) return;

      bool     ext;
      unsigned curses_colour = convertColourToCurses(colour, ext);

      if (ext)
         curses.extfgcolour(curses_colour);
      else
         curses.fgcolour(curses_colour);
   }

   //! Set background colour
   virtual void setBackgroundColour(Colour colour) override
   {
      if(!screen_enable) return;

      bool     ext;
      unsigned curses_colour = convertColourToCurses(colour, ext);

      if (ext)
         curses.extbgcolour(curses_colour);
      else
         curses.bgcolour(curses_colour);
   }

   //! Move cursor
   virtual void moveCursor(unsigned line, unsigned col) override
   {
      if(!screen_enable) return;

      curses.move(line, col);
   }

   //! Read character.
   //! \returns false on timeout
   virtual bool read(uint8_t& data, unsigned timeout_100ms) override
   {
      int ch;

      while(true)
      {
         ch = getInput(timeout_100ms);
         if(ch < 0)
         {
            exit(0); // TODO this seems a bit severe!
         }
         else if(ch == 0x7F)
         {
            ch = '\b';
            break;
         }
         else if(ch < 0x7F)
         {
            break;
         }
      }

      scroll           = 0;
      only_white_space = true;

      data = ch;

      return ch != 0;
   }

   //! Write character
   virtual void write(uint8_t ch) override
   {
      if(!screen_enable) return;

      curses.addch(ch);

      if(!isspace(ch))
      {
         only_white_space = false;
      }

      if((ch == '\n') && !isInputFileOpen())
      {
         scroll++;
         if(scroll == (curses.lines - 1))
         {
            waitForKey();
         }
      }
   }

   //! Wait for any key press
   virtual void waitForKey() override
   {
      if(only_white_space) return;

      curses.addstr("...");
      curses.getch();
      curses.addstr("\b\b\b   \b\b\b");

      scroll           = 0;
      only_white_space = true;
   }

private:
   bool openInputFile(const char* filename_);
   void closeInputFile();
   bool isInputFileOpen();

   int getInput(unsigned timeout_ms);

   //! Convert colour code to a curses colour index
   unsigned convertColourToCurses(Colour colour, bool& ext)
   {
      switch(colour)
      {
      case BLACK:       ext = false; return 0;
      case RED:         ext = false; return 1;
      case GREEN:       ext = false; return 2;
      case YELLOW:      ext = false; return 3;
      case BLUE:        ext = false; return 4;
      case MAGENTA:     ext = false; return 5;
      case CYAN:        ext = false; return 6;
      case WHITE:       ext = false; return 7;

      case DEFAULT:     ext = false; return 9;

      case LIGHT_GREY:  ext = true;  return 250;
      case MEDIUM_GREY: ext = true;  return 244;
      case DARK_GREY:   ext = true;  return 237;
      }

      ext = false;
      return 0;
   }

   static const unsigned COL_NRM_BASE = 0;
   static const unsigned COL_DEFAULT  = 9;
   static const unsigned COL_EXT_BASE = 0x100;

   TRM::Curses curses;
   unsigned    num_fonts_avail{1};
   bool        colours_avail{true};
   unsigned    scroll{0};
   bool        only_white_space{true};
   bool        screen_enable{true};
};

#endif
