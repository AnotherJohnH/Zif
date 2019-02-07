//------------------------------------------------------------------------------
// Copyright (c) 2016-2019 John D. Haughton
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

#ifndef ZTEXT_H
#define ZTEXT_H

#include <cstdint>
#include <functional>

#include "ZMemory.h"
#include "ZStream.h"

// See the Z specification section 3.

//! Decompressor for text
class ZText
{
public:
   using Writer = std::function<void(uint16_t)>;

private:
   enum State : uint8_t
   {
      NORMAL,
      IN_ABBR,
      ABBR_1,
      ABBR_2 = ABBR_1 + 1,
      ABBR_3 = ABBR_2 + 1,
      ZSCII_UPPER,
      ZSCII_LOWER
   };

   // Linkage
   const ZMemory& memory;

   // Configuration
   uint8_t  version{0};
   uint16_t abbr_table{0};

   // Decoder state
   State    state;
   uint8_t  shift_lock;
   uint8_t  shift;
   uint16_t zscii{0};

   void reset(State state_, uint8_t shift_)
   {
      state      = state_;
      shift_lock = shift_;
      shift      = shift_;
   }

   void decodeAbbr(const Writer& writer, unsigned index)
   {
      uint8_t save_shift = shift_lock;

      reset(IN_ABBR, /* shift */ 0);

      for(uint32_t addr = memory.codeWord(abbr_table + index * 2) * 2;
          decode(writer, memory.codeWord(addr));
          addr += 2);

      reset(NORMAL, save_shift);
   }

   void decodeZChar(const Writer& writer, uint8_t code)
   {
      // Alphabet table (v1) [3.5.4]
      static const char* alphabet_v1 = "abcdefghijklmnopqrstuvwxyz"    // A0
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"    // A1
                                       " 0123456789.,!?_#'\"/\\<-:()"; // A2

      // Alphabet table (v2 - v4) [3.5.3]
      static const char* alphabet_v2_v4 = "abcdefghijklmnopqrstuvwxyz"     // A0
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"     // A1
                                          " \n0123456789.,!?_#'\"/\\-:()"; // A2

      switch(state)
      {
      case ABBR_1:
      case ABBR_2:
      case ABBR_3:
         decodeAbbr(writer, (state - ABBR_1) * 32 + code);
         return;

      case ZSCII_UPPER:
         zscii = code;
         state = ZSCII_LOWER;
         return;

      case ZSCII_LOWER:
         writer((zscii << 5) | code);
         state = NORMAL;
         return;

      case NORMAL:
      case IN_ABBR:
         break;
      }

      switch(code)
      {
      case 0:
         // Z char 0 is a space [3.5.1]
         writer(' ');
         return;

      case 1:
         if(version == 1)
         {
            // Z char 1 is a new line (v1) [3.5.2]
            writer('\n');
         }
         else if(state == NORMAL)
         {
            // Abbreviation 0-31 (v2+) [3.3]
            state = ABBR_1;
         }
         break;

      case 2:
         if(version <= 2)
         {
            // Shift up (v1 and v2) [3.2.2]
            shift = (shift + 1) % 3;
         }
         else if(state == NORMAL)
         {
            // Abbreviation 32-63 (v3+) [3.3]
            state = ABBR_2;
         }
         return;

      case 3:
         if(version <= 2)
         {
            // Shift down (v1 and v2) [3.2.2]
            shift = (shift + 2) % 3;
         }
         else if(state == NORMAL)
         {
            // 3.3 Abbreviation 64-95 (v3+) [3.3]
            state = ABBR_3;
         }
         return;

      case 4:
         // Shift up [3.2.3]
         shift = (shift + 1) % 3;
         // Apply shift-lock (v1 and v2) [3.2.2]
         if (version < 3) shift_lock = shift;
         return;

      case 5:
         // Shift down [3.2.3]
         shift = (shift + 2) % 3;
         // Apply shift-lock (v1 and v2) [3.2.2]
         if (version < 3) shift_lock = shift;
         return;

      default:
         // [3.5]
         if(shift == 2)
         {
            if(code == 6)
            {
               state = ZSCII_UPPER;
               shift = shift_lock;
               break;
            }
            else if((code == 7) && (version != 1))
            {
               writer('\n');
               shift = shift_lock;
               break;
            }
         }

         {
            const char* table = nullptr;

            if(version == 1)
            {
               table = alphabet_v1;
            }
            else
            {
               table = alphabet_v2_v4;

               if(version >= 5)
               {
                  // TODO 3.5.5 check header for alternate table
               }
            }

            writer(table[(shift * 26) + code - 6]);
            shift = shift_lock;
         }
         break;
      }
   }

   //! Decode text packed into a 16bit word
   bool decode(const Writer& writer, uint16_t word)
   {
      decodeZChar(writer, (word >> 10) & 0x1F);
      decodeZChar(writer, (word >>  5) & 0x1F);
      decodeZChar(writer, (word >>  0) & 0x1F);

      bool cont = (word & (1 << 15)) == 0;
      return cont;
   }

public:
   ZText(ZMemory& memory_)
      : memory(memory_)
   {
   }

   //! Initialise
   void init(uint8_t version_, uint16_t abbr_table_)
   {
      version    = version_;
      abbr_table = abbr_table_;
   }

   //! Write packed text starting at the given address
   uint32_t print(const Writer& writer, uint32_t addr)
   {
      reset(NORMAL, /* shift */ 0);

      while(decode(writer, memory.codeWord(addr)))
      {
          addr += 2;
      }

      return addr + 2;
   }

   //! Write raw text starting at the given address
   void printTable(const Writer& writer,
                   uint32_t      addr,
                   unsigned      width,
                   unsigned      height,
                   unsigned      skip)
   {
      for(unsigned line = 0; line < height; line++)
      {
         for(unsigned col = 0; col < width; col++)
         {
            writer(memory.codeByte(addr++));
         }

         addr += skip;
      }
   }

   //! Write raw text starting at the given address
   void printForm(const Writer& writer, uint32_t addr)
   {
      while(true)
      {
         uint16_t length = memory.codeWord(addr);
         if (length == 0) break;
         addr += 2;

         for(unsigned i=0; i<length; i++)
         {
            writer(memory.codeByte(addr++));
         }
      }
   }
};

#endif
