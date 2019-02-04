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

#ifndef Z_SCREEN_H
#define Z_SCREEN_H

#include "Console.h"
#include "ZStream.h"

#define DBGF if (0) printf

//! Screen model
class ZScreen
{
private:
   static const unsigned MAX_WINDOW = 8;

   struct Vec
   {
      uint16_t x{0}, y{0};
   };

   struct ZWindow
   {
      Vec      pos;
      Vec      size;
      Vec      cursor;
      uint16_t left_margin{0};
      uint16_t right_margin{0};
      uint16_t newline_handler{0};
      uint16_t interrupt_countdown{0};
      uint8_t  text_style{0};
      uint16_t colour_data{0};
      uint8_t  font_number{0};
      uint8_t  font_size{0};
      uint8_t  attr{0};
      int16_t  line_count{0};
      bool     printer_enabled{false};
      bool     buffering{false};
   };

public:
   static const unsigned WINDOW_LOWER{0};
   static const unsigned WINDOW_UPPER{1};

   ZScreen(Console& console_, ZStream& stream_)
      : console(console_)
      , stream(stream_)
   {
   }

   ~ZScreen()
   {
      // console.waitForKey();
   }

   unsigned getWidth() const
   {
      return console.getAttr(Console::COLS);
   }

   unsigned getHeight() const
   {
      return console.getAttr(Console::LINES);
   }

   void getCursor(unsigned& row, unsigned &col) const
   {
      console.getCursorPos(row, col);
   }

   //! Initialise the screen state
   void init(unsigned version_)
   {
      DBGF("Screen::init(%u)\n", version_);

      version = version_;

      switch(version)
      {
      case 1:
      case 2:
         // 8.5.2 clear screen and move cursor to bottom left corner
         console.clear();
         console.moveCursor(getHeight(), 1);
         break;

      case 3:
         // 8.6.2 clear screen and move cursor to bottom left corner
         console.clear();
         console.moveCursor(getHeight(), 1);
         window[WINDOW_LOWER].pos.x  = 1;
         window[WINDOW_LOWER].pos.y  = getHeight();
         window[WINDOW_LOWER].size.x = getWidth();
         window[WINDOW_LOWER].size.y = getHeight();
         window[WINDOW_LOWER].printer_enabled = stream.getStreamEnable(2);
         break;

      case 4:
      case 5:
      case 7:
      case 8:
         eraseWindow(-1);
         break;

      case 6:
         eraseWindow(-1);
         break;
      }
   }

   // Update the status line (v1-3)
   void showStatus(const std::string& text)
   {
      DBGF("Screen::showStatus(\"%s\")\n", text.c_str());

      assert(version <= 3);

      bool printer_enabled = stream.getStreamEnable(2);
      stream.enableStream(2, false);

      unsigned row, col;
      console.getCursorPos(row, col);

      // Inverse video header bar
      console.setFontStyle(Console::FONT_STYLE_REVERSE);

      console.write(1, 1, text);

      // Restore cursor and style
      console.setFontStyle(0);
      console.moveCursor(row, col);

      stream.enableStream(2, printer_enabled);
   }

   // Split window (v3+)
   void splitWindow(unsigned upper_height_)
   {
      DBGF("Screen::splitWindow(%u)\n", upper_height_);

      assert(version >= 3);

      window[WINDOW_LOWER].pos.x  = 1;
      window[WINDOW_LOWER].pos.y  = upper_height_ + 1;
      window[WINDOW_LOWER].size.x = getWidth();
      window[WINDOW_LOWER].size.y = getHeight() - upper_height_;

      if (upper_height_ != 0)
      {
          window[WINDOW_UPPER].pos.x  = 1;
          window[WINDOW_UPPER].pos.y  = 1;
          window[WINDOW_UPPER].size.x = window[WINDOW_LOWER].size.x;
          window[WINDOW_UPPER].size.y = upper_height_;

          if (version == 3)
          {
             console.clearLines(1, upper_height_);
          }
      }
      else
      {
          window[WINDOW_UPPER].pos.x  = 0;
          window[WINDOW_UPPER].pos.y  = 0;
          window[WINDOW_UPPER].size.x = 0;
          window[WINDOW_UPPER].size.y = 0;
      }

      console.setScrollRegion(window[WINDOW_LOWER].pos.y,
                              window[WINDOW_LOWER].pos.y + window[WINDOW_LOWER].size.y);
   }

   // Select window (v3+)
   void selectWindow(unsigned index_)
   {
      DBGF("Screen::selectWindow(%u)\n", index_);

      assert(version >= 3);

      // Save state of current window
      ZWindow& current = window[index];

      unsigned line, col;
      console.getCursorPos(line, col);
      current.cursor.y        = line;
      current.cursor.x        = col;
      current.printer_enabled = stream.getStreamEnable(2);
      current.buffering       = stream.getBuffering();

      index = index_;

      // Set state for next window
      ZWindow& next = window[index];

      if (index == WINDOW_UPPER)
      {
         if (version != 6)
         {
            next.cursor.y        = 1;
            next.cursor.x        = 1;
            next.printer_enabled = false;
            next.buffering       = false;
         }
      }
      else if (index == WINDOW_LOWER)
      {
         if (version == 4)
         {
            next.cursor.y = next.pos.y + next.size.y - 1;
            next.cursor.x = 1;
         }
      }

      stream.setCol(next.cursor.x);
      stream.enableStream(2, next.printer_enabled);
      stream.setBuffering(next.buffering);

      console.moveCursor(next.cursor.y, next.cursor.x);
   }

   // Erase window (v4+)
   void eraseWindow(signed index)
   {
      DBGF("Screen::eraseWindow(%d)\n", index);

      assert(version >= 4);

      if (index == -1)
      {
         splitWindow(0);
         index = WINDOW_LOWER;
         selectWindow(index);
      }

      // TODO just clear the selected window
      //console.clearLines(window[index].pos.y, window[index].size.y);
      console.clear();
   }

   // Erase line (v4+)
   void eraseLine()
   {
      DBGF("Screen::eraseLine()\n");

      assert(version >= 4);

      console.eraseLine();
   }

   // Erase line (v4+)
   void moveCursor(unsigned row, unsigned col)
   {
      DBGF("Screen::moveCursor(%u, %u)\n", row, col);

      assert(version >= 4);

      if (version == 6)
      {
      }
      else if (version >= 4)
      {
         if (index == WINDOW_UPPER)
         {
            console.moveCursor(row, col);
         }
      }
   }

   //! Get a windows property (v6)
   uint16_t getWindowProp(unsigned index_, unsigned prop_) const
   {
      DBGF("Screen::getWindowProp(%u, %u)\n", index_, prop_);

      assert(version == 6);

      // TODO validate index and prop

      switch(prop_)
      {
      case  0: return window[index_].pos.y;
      case  1: return window[index_].pos.x;
      case  2: return window[index_].size.y;
      case  3: return window[index_].size.x;
      case  4: return window[index_].cursor.y;
      case  5: return window[index_].cursor.x;
      case  6: return window[index_].left_margin;
      case  7: return window[index_].right_margin;
      case  8: return window[index_].newline_handler;
      case  9: return window[index_].interrupt_countdown;
      case 10: return window[index_].text_style;
      case 11: return window[index_].colour_data;
      case 12: return window[index_].font_number;
      case 13: return window[index_].font_size;
      case 14: return window[index_].attr;
      case 15: return window[index_].line_count;

      default: return 0;
      }
   }

   //! Set a windows property (v6)
   void setWindowProp(unsigned index_, unsigned prop_, unsigned value)
   {
      DBGF("Screen::setWindowProp(%u, %u, %u)\n", index_, prop_, value);

      assert(version == 6);

      // TODO validate index and prop
      // TODO side effects

      switch(prop_)
      {
      case  0: window[index_].pos.y = value; break;
      case  1: window[index_].pos.x = value; break;
      case  2: window[index_].size.y = value; break;
      case  3: window[index_].size.x = value; break;
      case  4: window[index_].cursor.y = value; break;
      case  5: window[index_].cursor.x = value; break;
      case  6: window[index_].left_margin = value; break;
      case  7: window[index_].right_margin = value; break;
      case  8: window[index_].newline_handler = value; break;
      case  9: window[index_].interrupt_countdown = value; break;
      case 10: window[index_].text_style = value; break;
      case 11: window[index_].colour_data = value; break;
      case 12: window[index_].font_number = value; break;
      case 13: window[index_].font_size = value; break;
      case 14: window[index_].attr = value; break;
      case 15: window[index_].line_count = value; break;

      default: break;
      }
   }

private:
   Console&  console;
   ZStream&  stream;
   ZWindow   window[MAX_WINDOW];
   unsigned  version{0};
   unsigned  index{WINDOW_LOWER};
};

#endif
