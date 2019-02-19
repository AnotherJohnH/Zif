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

#include "share/Memory.h"

#include "ZStream.h"
#include "ZHeader.h"

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
   const IF::Memory& memory;

   // Configuration
   uint8_t     version{0};
   uint16_t    abbr_table{0};
   const char* alpha_table{nullptr};

   // Decoder state
   State    state;
   uint8_t  shift_lock;
   uint8_t  alphabet;
   uint16_t zscii{0};

   void reset(State state_, uint8_t alphabet_)
   {
      state      = state_;
      shift_lock = alphabet_;
      alphabet   = alphabet_;
   }

   void decodeAbbr(const Writer& writer, unsigned index)
   {
      uint8_t save_shift = shift_lock;
      uint32_t abbr_addr = memory.fetch16(abbr_table + index * 2) * 2;

      decodeText(writer, IN_ABBR, abbr_addr);
      reset(NORMAL, save_shift);
   }

   void decodeZChar(const Writer& writer, uint8_t zchar)
   {
      switch(state)
      {
      case ABBR_1:
      case ABBR_2:
      case ABBR_3:
         decodeAbbr(writer, (state - ABBR_1) * 32 + zchar);
         return;

      case ZSCII_UPPER:
         zscii = zchar;
         state = ZSCII_LOWER;
         return;

      case ZSCII_LOWER:
         writer((zscii << 5) | zchar);
         state = NORMAL;
         return;

      case NORMAL:
      case IN_ABBR:
         break;
      }

      switch(zchar)
      {
      case 0:
         // Z char 0 is a space [3.5.1]
         writer(' ');
         break;

      case 1:
         if(version == 1)
         {
            // Z char 1 is a new line (v1) [3.5.2]
            writer('\n');
         }
         else if(state == NORMAL) // [3.3.1]
         {
            // Abbreviation 0-31 (v2+) [3.3]
            state = ABBR_1;
         }
         break;

      case 2:
         if(version <= 2)
         {
            // Shift up (v1 and v2) [3.2.2]
            alphabet = (alphabet + 1) % 3;
         }
         else if(state == NORMAL) // [3.3.1]
         {
            // Abbreviation 32-63 (v3+) [3.3]
            state = ABBR_2;
         }
         break;

      case 3:
         if(version <= 2)
         {
            // Shift down (v1 and v2) [3.2.2]
            alphabet = (alphabet + 2) % 3;
         }
         else if(state == NORMAL) // [3.3.1]
         {
            // 3.3 Abbreviation 64-95 (v3+) [3.3]
            state = ABBR_3;
         }
         break;

      case 4:
         // Shift up [3.2.2]
         alphabet = (alphabet + 1) % 3;
         // Apply shift-lock (v1 and v2) [3.2.2, 3.2.3]
         if (version < 3) shift_lock = alphabet;
         break;

      case 5:
         // Shift down [3.2.2]
         alphabet = (alphabet + 2) % 3;
         // Apply shift-lock (v1 and v2) [3.2.2, 3.2.3]
         if (version < 3) shift_lock = alphabet;
         break;

      default:
         if(alphabet == 2)
         {
            if(zchar == 6)
            {
               // [3.4]
               state    = ZSCII_UPPER;
            }
            else if((zchar == 7) && (version != 1))
            {
               writer('\n');
            }
            else
            {
               writer(alpha_table[(alphabet * 26) + zchar - 6]);
            }
         }
         else
         {
            writer(alpha_table[(alphabet * 26) + zchar - 6]);
         }
         alphabet = shift_lock;
         break;
      }
   }

   //! Decode text packed into a 16bit word
   bool defetch16(const Writer& writer, uint16_t word)
   {
      decodeZChar(writer, (word >> 10) & 0x1F);
      decodeZChar(writer, (word >>  5) & 0x1F);
      decodeZChar(writer, (word >>  0) & 0x1F);

      bool cont = (word & (1 << 15)) == 0;
      return cont;
   }

   //! Decode text starting at the given address
   uint32_t decodeText(const Writer& writer, State state_, uint32_t addr)
   {
      // Start with alphabet A0 [3.2.1]
      reset(state_, /* alphabet */ 0);

      while(defetch16(writer, memory.fetch16(addr)))
      {
          addr += 2;
      }

      return addr + 2;
   }

public:
   ZText(const ZHeader* header, IF::Memory& memory_)
      : memory(memory_)
   {
      version    = header->version;
      abbr_table = header->abbr;

      if(version == 1)
      {
         // Alphabet table (v1) [3.5.4]
         alpha_table = "abcdefghijklmnopqrstuvwxyz"    // A0
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"    // A1
                       " 0123456789.,!?_#'\"/\\<-:()"; // A2
      }
      else if((version >= 5) && (header->alphabet_table != 0))
      {
         // Check header for alternate table [3.5.5]
         alpha_table = (const char*)memory.data() + header->alphabet_table;
      }
      else
      {
         // Alphabet table (v2 - v4) [3.5.3]
         alpha_table = "abcdefghijklmnopqrstuvwxyz"     // A0
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"     // A1
                       " \n0123456789.,!?_#'\"/\\-:()"; // A2
      }
   }

   //! Write packed text starting at the given address
   uint32_t print(const Writer& writer, uint32_t addr)
   {
      return decodeText(writer, NORMAL, addr);
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
            writer(memory.fetch8(addr++));
         }

         addr += skip;
      }
   }

   //! Write raw text starting at the given address
   void printForm(const Writer& writer, uint32_t addr)
   {
      while(true)
      {
         uint16_t length = memory.fetch16(addr);
         if (length == 0) break;
         addr += 2;

         for(unsigned i=0; i<length; i++)
         {
            writer(memory.fetch8(addr++));
         }
      }
   }
};

#endif
