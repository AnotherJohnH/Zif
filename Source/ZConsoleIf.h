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

#ifndef Z_CONSOLE_IF_H
#define Z_CONSOLE_IF_H

#include <cstdint>

//! Console interface 
class ZConsoleIf
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

   static const unsigned ATTR_REVERSE = 1 << 0;
   static const unsigned ATTR_BOLD    = 1 << 1;
   static const unsigned ATTR_ITALIC  = 1 << 2;
   static const unsigned ATTR_FIXED   = 1 << 3;

   void setExtendedColours(bool state)
   {
      extended_colours = state;
   }

   //! Return console attribute
   virtual unsigned getAttr(Attr attr) const = 0;

   //! Get current position of cursor
   virtual void getCursorPos(unsigned& line, unsigned& col) = 0;

   //! Clear the console
   virtual void clear() = 0;

   //! Select the current font
   virtual bool setFont(unsigned font_idx) = 0;

   //! Set (curses format) attributes
   virtual void setAttributes(unsigned attr) = 0;

   //! Set foreground and background colours
   virtual void setColours(signed fg, signed bg) = 0;

   //! Move cursor
   virtual void moveCursor(unsigned line, unsigned col) = 0;

   //! Read ZSCII character.
   //! Returns false on timeout
   virtual bool read(uint16_t& zscii, unsigned timeout_100ms) = 0;

   //! Write ZSCII character
   virtual void write(uint16_t zscii) = 0;

   //! Wait for any key press
   virtual void waitForKey() = 0;

protected:
   bool extended_colours{false};
};

#endif
