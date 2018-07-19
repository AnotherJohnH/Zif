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

#ifndef ZTEXT_H
#define ZTEXT_H

#include <cstdint>

#include "ZMemory.h"
#include "ZStream.h"

// See the Z specification section 3.

//! Decompressor for text
class ZText
{
private:
   enum State : uint8_t
   {
      NORMAL,
      DECODE_ABBR,
      ABBR_1,
      ABBR_2,
      ABBR_3,
      ASCII_UPPER,
      ASCII_LOWER
   };

   // Linkage
   ZStream&       stream;
   const ZMemory& memory;

   // Configuration
   uint8_t  version;
   uint16_t abbr;

   // Decoder state
   State   state;
   uint8_t a;
   uint8_t next_a;
   uint8_t ascii;

   void decodeAbbr(unsigned index)
   {
      uint32_t entry = memory.readWord(abbr + index * 2) * 2;

      state = DECODE_ABBR;

      for(; decode(memory.readWord(entry)); entry += 2)
      {
      }
   }

   void resetDecoder()
   {
      state  = NORMAL;
      a      = 0;
      next_a = 0;
      ascii  = 0;
   }

   void decodeZChar(uint8_t code)
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
         decodeAbbr((state - ABBR_1) * 32 + code);
         a     = next_a;
         state = NORMAL;
         return;

      case ASCII_UPPER:
         ascii = code;
         state = ASCII_LOWER;
         return;

      case ASCII_LOWER:
         stream.writeChar((ascii << 5) | code);
         state = NORMAL;
         return;

      case NORMAL:
      case DECODE_ABBR:
         break;
      }

      switch(code)
      {
      case 0:
         // Z char 0 is a space [3.5.1]
         stream.writeChar(' ');
         return;

      case 1:
         if(version == 1)
         {
            // Z char 1 is a new line (v1) [3.5.2]
            stream.writeChar('\n');
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
            next_a = a;
            a      = (a + 1) % 3;
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
            next_a = a;
            a      = (a + 2) % 3;
         }
         else if(state == NORMAL)
         {
            // 3.3 Abbreviation 64-95 (v3+) [3.3]
            state = ABBR_3;
         }
         return;

      case 4:
         // Shift up [3.2.3]
         a = (a + 1) % 3;
         // Apply shift-lock (v1 and v2) [3.2.2]
         next_a = version < 3 ? a : 0;
         return;

      case 5:
         // Shift down [3.2.3]
         a = (a + 2) % 3;
         // Apply shift-lock (v1 and v2) [3.2.2]
         next_a = version < 3 ? a : 0;
         return;

      default:
         // [3.5]
         if(a == 2)
         {
            if(code == 6)
            {
               state = ASCII_UPPER;
               break;
            }
            else if((code == 7) && (version != 1))
            {
               stream.writeChar('\n');
               break;
            }
         }

         {
            const char* table = alphabet_v2_v4;

            if(version == 1)
            {
               table = alphabet_v1;
            }
            else if(version >= 5)
            {
            }

            stream.writeChar(table[(a * 26) + code - 6]);
         }
         break;
      }

      a = next_a;
   }

   //! Decode text packed into a 16bit word
   bool decode(uint16_t word)
   {
      bool cont = (word & (1 << 15)) == 0;

      decodeZChar((word >> 10) & 0x1F);
      decodeZChar((word >>  5) & 0x1F);
      decodeZChar((word >>  0) & 0x1F);

      return cont;
   }

public:
   ZText(ZStream& stream_, ZMemory& memory_)
      : stream(stream_)
      , memory(memory_)
      , version(0)
      , abbr(0)
   {
   }

   //! Initialise
   void init(uint8_t version_, uint32_t abbr_)
   {
      version = version_;
      abbr    = abbr_;
   }

   //! Write packed text starting at the given address
   uint32_t print(uint32_t addr)
   {
      resetDecoder();

      while(decode(memory.readWord(addr)))
      {
          addr += 2;
      }

      return addr + 2;
   }

   //! Write raw text starting at the given address
   void printTable(uint32_t addr, unsigned width, unsigned height, unsigned skip)
   {
      for(unsigned line = 0; line < height; line++)
      {
         for(unsigned col = 0; col < width; col++)
         {
            stream.writeChar(memory[addr++]);
         }

         addr += skip;
      }
   }
};

#endif
