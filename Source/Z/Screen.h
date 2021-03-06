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

#include "common/Console.h"

#include "Z/Stream.h"

#define DBGF if (0) printf

namespace Z {

//! Screen model
class Screen
{
private:
   static const unsigned LOWER_WINDOW{0};
   static const unsigned UPPER_WINDOW{1};
   static const unsigned MAX_WINDOW{8};

   struct Vec
   {
      uint16_t x{0}, y{0};

      Vec(uint16_t x_, uint16_t y_) : x(x_), y(y_) {}
   };

   struct ZWindow
   {
      Vec      pos{1,1};
      Vec      size{0,0};
      Vec      cursor{1,1};
      uint16_t left_margin{0};
      uint16_t right_margin{0};
      uint16_t newline_handler{0};
      uint16_t interrupt_countdown{0};
      uint8_t  text_style{0};
      uint16_t colour_data{0};
      uint8_t  font_number{0};
      uint8_t  font_size{1};
      uint8_t  attr{0};
      int16_t  line_count{0};
      bool     printer_enabled{false};
      bool     buffering{false};
   };

public:
   Screen(Console& console_, Stream& stream_, unsigned version_)
      : console(console_)
      , stream(stream_)
      , version(version_)
   {
   }

   ~Screen()
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
   void reset()
   {
      switch(version)
      {
      case 1:
      case 2:
         // 8.5.2 clear screen and move cursor to bottom left corner
         console.clear();
         console.moveCursor(getHeight(), 1);
         stream.setCol(1);
         break;

      case 3:
         // 8.6.2 clear screen and move cursor to bottom left corner
         console.clear();
         console.moveCursor(getHeight(), 1);
         stream.setCol(1);
         window[LOWER_WINDOW].pos.x  = 1;
         window[LOWER_WINDOW].pos.y  = getHeight();
         window[LOWER_WINDOW].size.x = getWidth();
         window[LOWER_WINDOW].size.y = getHeight();
         window[LOWER_WINDOW].printer_enabled = stream.getStreamEnable(2);
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

      console.moveCursor(1, 1);
      console.writeString(text);

      // Restore cursor and style
      console.setFontStyle(0);
      console.moveCursor(row, col);
      stream.setCol(col);

      stream.enableStream(2, printer_enabled);
   }

   // Split window (v3+)
   void splitWindow(unsigned upper_height_)
   {
      DBGF("Screen::splitWindow(%u)\n", upper_height_);

      assert(version >= 3);

      window[LOWER_WINDOW].pos.x  = 1;
      window[LOWER_WINDOW].pos.y  = upper_height_ + 1;
      window[LOWER_WINDOW].size.x = getWidth();
      window[LOWER_WINDOW].size.y = getHeight() - upper_height_;

      if (upper_height_ != 0)
      {
          window[UPPER_WINDOW].pos.x  = 1;
          window[UPPER_WINDOW].pos.y  = 1;
          window[UPPER_WINDOW].size.x = window[LOWER_WINDOW].size.x;
          window[UPPER_WINDOW].size.y = upper_height_;

          if (version == 3)
          {
             console.clearLines(1, upper_height_);
          }
      }
      else
      {
          window[UPPER_WINDOW].pos.x  = 0;
          window[UPPER_WINDOW].pos.y  = 0;
          window[UPPER_WINDOW].size.x = 0;
          window[UPPER_WINDOW].size.y = 0;
      }

      console.setScrollRegion(window[LOWER_WINDOW].pos.y,
                              window[LOWER_WINDOW].pos.y + window[LOWER_WINDOW].size.y);
   }

   // Select window (v3+)
   void selectWindow(unsigned index_)
   {
      DBGF("Screen::selectWindow(%u)\n", index_);

      assert(version >= 3);

      stream.flush();

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

      if (index == UPPER_WINDOW)
      {
         if (version != 6)
         {
            next.cursor.y        = 1;
            next.cursor.x        = 1;
            next.printer_enabled = false;
            next.buffering       = false;
         }
      }
      else if (index == LOWER_WINDOW)
      {
         if (version == 4)
         {
            next.cursor.y = next.pos.y + next.size.y - 1;
            next.cursor.x = 1;
         }
         else
         {
            if (next.cursor.y < next.pos.y)
               next.cursor.y = next.pos.y;
            else if (next.cursor.y >= (next.pos.y + next.size.y))
               next.cursor.y = next.pos.y + next.size.y - 1;;

            if (next.cursor.x < next.pos.x)
               next.cursor.x = next.pos.x;
            else if (next.cursor.x >= (next.pos.x + next.size.x))
               next.cursor.x = next.pos.x + next.size.x - 1;;
         }
      }

      stream.enableStream(2, next.printer_enabled);
      stream.setBuffering(next.buffering);

      stream.setCol(next.cursor.x);
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
         index = LOWER_WINDOW;
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

   // Move cursor (v4+)
   void moveCursor(signed row, unsigned col, unsigned index_ = 0)
   {
      DBGF("Screen::moveCursor(%u, %u)\n", row, col);

      assert(version >= 4);

      if (version == 6)
      {
         if (row == -1)
         {
            console.setCursorVisibility(false);
         }
         else if (row == -2)
         {
            console.setCursorVisibility(true);
         }
         else
         {
            window[index_].cursor.x = window[index_].pos.x - 1 + col;
            window[index_].cursor.y = window[index_].pos.y - 1 + row;

            if (index == index_)
            {
               stream.setCol(window[index].cursor.x);
               console.moveCursor(window[index_].cursor.y,
                                  window[index_].cursor.x);
            }
         }
      }
      else if (version >= 4)
      {
         if (index == UPPER_WINDOW)
         {
            stream.setCol(col);
            console.moveCursor(row, col);
         }
      }
   }

   //! Get a windows property (v6)
   uint16_t getWindowProp(unsigned index_, unsigned prop_) const
   {
      DBGF("Screen::getWindowProp(%u, %u)", index_, prop_);

      assert(version == 6);

      uint16_t value = 0;

      if (index_ < MAX_WINDOW)
      {
          switch(prop_)
          {
          case  0: value = window[index_].pos.y; break;
          case  1: value = window[index_].pos.x; break;
          case  2: value = window[index_].size.y; break;
          case  3: value = window[index_].size.x; break;
          case  4: value = window[index_].cursor.y; break;
          case  5: value = window[index_].cursor.x; break;
          case  6: value = window[index_].left_margin; break;
          case  7: value = window[index_].right_margin; break;
          case  8: value = window[index_].newline_handler; break;
          case  9: value = window[index_].interrupt_countdown; break;
          case 10: value = window[index_].text_style; break;
          case 11: value = window[index_].colour_data; break;
          case 12: value = window[index_].font_number; break;
          case 13: value = window[index_].font_size; break;
          case 14: value = window[index_].attr; break;
          case 15: value = window[index_].line_count; break;
          default: break;
          }
      }

      DBGF(" => %u\n", value);

      return value;
   }

   //! Set a windows property (v6)
   void setWindowProp(unsigned index_, unsigned prop_, unsigned value)
   {
      DBGF("Screen::setWindowProp(%u, %u, %u)\n", index_, prop_, value);

      assert(version == 6);

      if (index_ < MAX_WINDOW)
      {
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
   }

   // move window to pixel position (v6)
   void moveWindow(unsigned index_, unsigned x_, unsigned y_)
   {
      DBGF("Screen::moveWindow(%u, %u, %u)\n", index_, x_, y_);

      assert(version == 6);

      if (index_ < MAX_WINDOW)
      {
          // TODO pixels
          window[index_].pos.x = x_;
          window[index_].pos.y = y_;
      }
   }

   // Change size of window in pixels (v6)
   void resizeWindow(unsigned index_, unsigned width_, unsigned height_)
   {
      DBGF("Screen::resizeWindow(%u, %u, %u)\n", index_, width_, height_);

      assert(version == 6);

      if (index_ < MAX_WINDOW)
      {
          // TODO pixels
          window[index_].size.x = width_;
          window[index_].size.y = height_;
      }
   }

   // Change the attributes for a given window (v6)
   void setWindowStyle(unsigned index_, unsigned flags_, unsigned operation_)
   {
      DBGF("Screen::setWindowStyle(%u, %x, %u)\n", index_, flags_, operation_);

      assert(version == 6);

      if (index_ < MAX_WINDOW)
      {
          // TODO
      }
   }

   // scroll window in pixels (v6)
   void scrollWindow(unsigned index_, unsigned pixels_)
   {
      DBGF("Screen::scrollWindow(%u, %u)\n", index_, pixels_);

      assert(version == 6);

      if (index_ < MAX_WINDOW)
      {
          // TODO
      }
   }

private:
   Console&  console;
   Stream&   stream;
   unsigned  version{0};
   ZWindow   window[MAX_WINDOW];
   unsigned  index{LOWER_WINDOW};
};

} // namespace Z

#endif
