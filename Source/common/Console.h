//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

//! Console interface
class Console
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

   enum Colour
   {
      BLACK,
      RED,
      GREEN,
      YELLOW,
      BLUE,
      MAGENTA,
      CYAN,
      WHITE,
      DEFAULT,
      LIGHT_GREY,
      MEDIUM_GREY,
      DARK_GREY
   };

   using FontStyle = uint8_t;

   static const FontStyle FONT_STYLE_NORMAL  = 0;
   static const FontStyle FONT_STYLE_REVERSE = 1 << 0;
   static const FontStyle FONT_STYLE_BOLD    = 1 << 1;
   static const FontStyle FONT_STYLE_ITALIC  = 1 << 2;
   static const FontStyle FONT_STYLE_FIXED   = 1 << 3;

   //! Return console attribute
   virtual unsigned getAttr(Attr attr) const = 0;

   //! Get current cursor position
   virtual void getCursorPos(unsigned& line, unsigned& col) = 0;

   //! Select the current font
   virtual bool setFont(unsigned font_idx) = 0;

   //! Set font style
   virtual void setFontStyle(FontStyle style_bit_mask) = 0;

   //! Set background colour
   virtual void setBackgroundColour(Colour colour) = 0;

   //! Set foreground colours
   virtual void setForegroundColour(Colour colour) = 0;

   //! Set cursor visibility
   virtual void setCursorVisibility(bool visible) = 0;

   //! Move the cursor
   virtual void moveCursor(unsigned line, unsigned col) = 0;

   //! Erase from current cursor position to the end of the line
   virtual void eraseLine() = 0;

   //! Wait for any key press
   virtual void waitForKey() = 0;

   //! Read character.
   //! Returns false on timeout
   virtual bool read(uint8_t& ch, unsigned timeout_ms) = 0;

   //! Set the scroll region
   virtual void setScrollRegion(unsigned top, unsigned bottom) = 0;

   //! Clear range of lines
   virtual void clearLines(unsigned first, unsigned n) = 0;

   //! Clear the console
   virtual void clear() = 0;

   //! Write character
   virtual void write(uint8_t ch) = 0;

   //! Write string
   void writeString(const std::string& text)
   {
      for(const auto& ch : text)
      {
         write(ch);
      }
   }

   //! Report an error
   void error(const std::string& text)
   {
      writeString("ERR: ");
      writeString(text);
      write('\n');
   }
};

