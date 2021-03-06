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

#ifndef Z_STREAM_H
#define Z_STREAM_H

#include <cassert>

#include "common/Console.h"
#include "common/Log.h"
#include "common/Options.h"
#include "common/Memory.h"

namespace Z {

//! Input/output stream handler
class Stream
{
public:
   enum MessageLevel
   {
      INFO,
      WARNING,
      ERROR
   };

   Stream(Console&       console_,
          const Options& options_,
          unsigned       version_,
          IF::Memory&    memory_)
      : console(console_)
      , memory(memory_)
   {
      console_enable                  = true;
      console_extended_colours_enable = version_ == 6;

      printer_enable     = options_.print;
      printer_echo_input = version_ <= 5;

      memory_enable  = false;
      snooper_enable = options_.key;

      buffer_enable = true;
      buffer_size   = 0;
      buffer_col    = 1;

      if(options_.info)
      {
         message_filter = INFO;
      }
      else if(options_.warn)
      {
         message_filter = WARNING;
      }

      trace_enable = options_.trace;
   }

   bool getBuffering() const { return buffer_enable; }

   bool getStreamEnable(unsigned index) const
   {
      switch(index)
      {
      case 1: return console_enable;
      case 2: return printer_enable;
      case 3: return memory_enable;
      case 4: return snooper_enable;

      default: assert(!"unexpected index"); return false;
      }
   }

   //! Select the current font
   bool setFont(unsigned font_idx)
   {
      flush();

      return console.setFont(font_idx);
   }

   //! Set Z-code text style
   void setFontStyle(unsigned text_style)
   {
      flush();

      if (text_style == 0)
      {
         console_text_style = 0;
      }
      else
      {
         console_text_style |= uint8_t(text_style);
      }

      // Convert Z-code text style to a Console font style. Might be a 1-1
      // mapping of bits, but copying each bit is more robust
      Console::FontStyle font_style = 0;
      if(console_text_style & (1 << 0)) font_style |= Console::FONT_STYLE_REVERSE;
      if(console_text_style & (1 << 1)) font_style |= Console::FONT_STYLE_BOLD;
      if(console_text_style & (1 << 2)) font_style |= Console::FONT_STYLE_ITALIC;
      if(console_text_style & (1 << 3)) font_style |= Console::FONT_STYLE_FIXED;

      console.setFontStyle(font_style);
   }

   //! Set colours
   void setColours(signed fg, signed bg)
   {
      flush();

      Console::Colour fg_col;
      if(convertCodeToConsoleColour(fg, fg_col))
      {
         console.setForegroundColour(fg_col);
      }

      Console::Colour bg_col;
      if(convertCodeToConsoleColour(bg, bg_col))
      {
         console.setBackgroundColour(bg_col);
      }
   }

   //! Synchronise current col used for line breaking
   void setCol(unsigned col_)
   {
      buffer_col = col_;
   }

   //! Control automatic line breaking
   void setBuffering(bool buffer_enable_)
   {
      flush();
      buffer_enable = buffer_enable_;
   }

   //! Flush any output that has been buffered
   void flush()
   {
      // TODO is the test required? flush elsewhere in this module
      //      is not conditional
      if(buffer_enable) flushOutputBuffer();
   }

   //! Control state of output streams
   void enableStream(unsigned index, bool next = true)
   {
      switch(index)
      {
      case 1: console_enable = next; break;
      case 2: printer_enable = next; break;
      case 3: memory_enable  = next; break;
      case 4: snooper_enable = next; break;

      default: assert(!"unexpected index"); break;
      }
   }

   //! Enable use of memory stream
   void enableMemoryStream(uint32_t ptr_, int16_t width_)
   {
      memory_len_ptr = ptr_;
      memory_ptr     = ptr_ + 2;
      memory_width   = width_;
      memory_enable  = true;

      memory.write16(memory_len_ptr, 0);
   }

   //! Read ZSCII character
   bool readChar(uint16_t& zscii, unsigned timeout_100ms, bool echo)
   {
      flushOutputBuffer();

      uint8_t ch;

      while(true)
      {
         if (console.read(ch, timeout_100ms * 100))
         {
            if (ch == '\b')
            {
               break;
            }
            else if ((ch == '\n') ||
                     (ch == 27) ||
                     ((ch >= 32) && (ch <= 126)) ||
                     ((ch >= 129) && (ch <= 254)))
            {
               if(echo)
               {
                  // Echo input to enabled output streams
                  if(console_enable)                       console.write(ch);
                  if(printer_enable && printer_echo_input) print(ch);
                  if(snooper_enable)                       snooper.write(ch);
                  if(trace_enable)                         trace_log.writePart("IN <= \"", ch, "\"\n");

                  if(ch == '\n') buffer_col = 1;
               }
               break;
            }
         }
         else
         {
            // Timeout
            return false;
         }
      }

      zscii = ch;
      return true;
   }

   //! Write ZSCII character (may be buffered)
   void writeChar(uint16_t zscii)
   {
      if(memory_enable)
      {
         memory.write16(memory_len_ptr, memory.read16(memory_len_ptr) + 1);
         memory.write8(memory_ptr++, zscii);
      }
      else if(buffer_enable)
      {
         sendBuffered(zscii);
      }
      else
      {
         send(zscii);
      }

      if(trace_enable)   trace_log.writePart("OUT => \"", char(zscii), "\"\n");
   }

   //! Delete the last character written
   void deleteChar()
   {
      send('\b');
      send(' ');
      send('\b');
   }

   //! Write signed integer value (may be buffered)
   void writeNumber(int16_t value_)
   {
      int32_t value = value_; // So that 32768 can be represented

      if(value < 0)
      {
         writeChar('-');
         value = -value;
      }

      int place = 10000;

      while((place > 1) && (value < place))
      {
         place = place / 10;
      }

      for(; place; place = place / 10)
      {
         unsigned digit = value / place;

         writeChar('0' + digit);

         value -= place * digit;
      }
   }

   void error(const std::string& text)
   {
      message(ERROR, text);
   }

   void warning(const std::string& text)
   {
      message(WARNING, text);
   }

   void info(const std::string& text)
   {
      message(INFO, text);
   }

   void trace(const std::string& string)
   {
      trace_log.write(string);
   }

   Log& getTrace()
   {
      return trace_log;
   }

private:
   static const unsigned MAX_WORD_LENGTH       = 16;
   static const unsigned PRINTER_NEWLINE_LIMIT = 3;

   //! Unbuffered write of a ZSCII character
   void send(uint16_t zscii)
   {
      if((zscii < ' ') || (zscii > 0x7E))
      {
         // Filter undefined output codes
         switch(zscii)
         {
         case '\0': return; // ignore
         case '\t': break;
         case '\n': break;
         case '\b': break;
         case '\r': zscii = '\n'; break;
         case 0x11: zscii = ' '; break; // v6 sentence space

         default:
            // "extra" characters
            if((zscii >= 155) && (zscii <=251))
            {
               zscii = '?';
               break;
            }
            return;
         }
      }

      uint8_t ch = uint8_t(zscii);
      if(ch == '\n')
         buffer_col = 1;
      else
         buffer_col++;

      if(console_enable) console.write(ch);
      if(printer_enable) print(ch);
   }

   //! Write raw string with no buffering
   void sendString(const char* s)
   {
      for(; *s; s++)
      {
         send(*s);
      }
   }

   //! Flush any output that has been buffered
   void flushOutputBuffer()
   {
      for(unsigned i = 0; i < buffer_size; i++)
      {
         send(buffer[i]);
      }

      buffer_size = 0;
   }

   //! Buffered write of an output character
   void sendBuffered(uint16_t zscii)
   {
      if((zscii == ' ') || (zscii == '\n') || (buffer_size == MAX_WORD_LENGTH))
      {
         if((buffer_col + buffer_size) > console.getAttr(Console::COLS))
         {
            send('\n');
         }

         flushOutputBuffer();

         send(zscii);
      }
      else
      {
         buffer[buffer_size++] = zscii;
      }
   }

   //! Send ZSCII character to the printer
   void print(uint16_t zscii)
   {
      // Filter repeated new-line
      if(zscii == '\r')
      {
         zscii = '\n';
      }
      if(zscii == '\n')
      {
         if(++printer_newline_count >= PRINTER_NEWLINE_LIMIT) return;
      }
      else
      {
         printer_newline_count = 0;
      }

      printer.write(zscii);
   }

   // Convert Z colour code to a console colour index
   bool convertCodeToConsoleColour(signed code, Console::Colour& colour)
   {
      // clang-format off
      switch(code)
      {
      case 0: return false; // No change
      case 1: colour = Console::DEFAULT; return true;
      case 2: colour = Console::BLACK;   return true;
      case 3: colour = Console::RED;     return true;
      case 4: colour = Console::GREEN;   return true;
      case 5: colour = Console::YELLOW;  return true;
      case 6: colour = Console::BLUE;    return true;
      case 7: colour = Console::MAGENTA; return true;
      case 8: colour = Console::CYAN;    return true;
      case 9: colour = Console::WHITE;   return true;
      default: break;
      }

      if(console_extended_colours_enable)
      {
         switch(code)
         {
         case -1: return false; // TODO colour of pixel under cursor
         case 10: colour = Console::LIGHT_GREY;  return true;
         case 11: colour = Console::MEDIUM_GREY; return true;
         case 12: colour = Console::DARK_GREY;   return true;
         default: break;
         }
      }
      // clang-format on

      return false;
   }

   //! Report a message
   void message(MessageLevel level, const std::string& text)
   {
      if(level < message_filter) return;

      // Start message on a new-line
      unsigned line, col;
      console.getCursorPos(line, col);
      if(col != 1)
      {
         send('\n');
      }

      // Identify message source
      console.setFontStyle(Console::FONT_STYLE_REVERSE);
      sendString("ZIF");
      console.setFontStyle(Console::FONT_STYLE_NORMAL);

      switch(level)
      {
      case INFO:    sendString(" "); break;
      case WARNING: sendString(" WRN: "); break;
      case ERROR:   sendString(" ERR: "); break;
      }

      sendString(text.c_str());
      send('\n');
   }

   // Console stream state
   bool     console_enable{true};
   bool     console_extended_colours_enable{false};
   uint8_t  console_text_style{0};
   Console& console;

   // Buffer used for automatic line breaks
   bool     buffer_enable{false};
   uint8_t  buffer_size{0};
   uint8_t  buffer[MAX_WORD_LENGTH];
   unsigned buffer_col{1};

   // Printer stream state
   bool     printer_enable{false};
   bool     printer_echo_input{false};
   unsigned printer_newline_count{1};
   Log      printer{"print.log"};

   // Memory stream state
   bool        memory_enable{false};
   int16_t     memory_width;
   uint32_t    memory_len_ptr;
   uint32_t    memory_ptr;
   IF::Memory& memory;

   // Input snooper stream state
   bool snooper_enable{false};
   Log  snooper{"key.log"};

   // Debug state
   bool trace_enable{false};
   Log  trace_log{"trace.log"};

   MessageLevel message_filter{ERROR};
};

} // namespace Z

#endif
