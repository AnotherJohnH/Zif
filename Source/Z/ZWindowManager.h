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

#ifndef Z_WINDOW_MANAGER_H
#define Z_WINDOW_MANAGER_H

#include "Console.h"
#include "Options.h"
#include "ZStream.h"

//! Console window manager
class ZWindowManager
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
   };

public:
   static const unsigned WINDOW_LOWER{0};
   static const unsigned WINDOW_UPPER{1};

   ZWindowManager(Console& console_, Options& options, ZStream& stream_)
      : console(console_)
      , stream(stream_)
   {
       printer_enabled = options.print;
   }

   uint16_t getWindowProp(unsigned index_, unsigned prop_)
   {
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

   void setWindowProp(unsigned index_, unsigned prop_, unsigned value)
   {
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

   // Update the status line (v1-3)
   void showStatus(const char* left, const char* right)
   {
      printer_enabled = stream.enableStream(2, false);

      console.moveCursor(1, 1);

      for(unsigned i = 0; i < console.getAttr(Console::COLS); i++)
      {
         if(left[i] == '\0') break;

         console.write(left[i]);
      }

      console.moveCursor(1, 999);

      stream.enableStream(2, printer_enabled);
   }

   void split(unsigned upper_height_) { window[WINDOW_UPPER].size.y = upper_height_; }

   void select(unsigned index_)
   {
      if(index == index_) return;

      index = index_;

      if(index == WINDOW_UPPER)
      {
         printer_enabled = stream.enableStream(2, false);

         unsigned line, col;
         console.getCursorPos(line, col);
         window[WINDOW_LOWER].cursor.y = line;
         window[WINDOW_LOWER].cursor.x = col;
         lower_buffering               = stream.setBuffering(false);
         stream.setCol(1);
         console.moveCursor(1, 1);
      }
      else
      {
         stream.enableStream(2, printer_enabled);

         console.moveCursor(window[WINDOW_LOWER].cursor.y, window[WINDOW_LOWER].cursor.x);
         stream.setBuffering(lower_buffering);
         stream.setCol(window[WINDOW_LOWER].cursor.x);
      }
   }

   void eraseWindow(unsigned index_)
   {
      // TODO properly
      console.clear();
   }

private:
   Console&  console;
   ZStream&  stream;
   ZWindow   window[MAX_WINDOW];
   unsigned  index{WINDOW_LOWER};
   bool      lower_buffering{true};
   bool      printer_enabled{false};
};

#endif
