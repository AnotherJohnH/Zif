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

#ifndef ZHEADER_H
#define ZHEADER_H

#include <cassert>
#include <cstdint>
#include <cstdio>

#include "STB/Endian.h"

#include "ZConfig.h"
#include "ConsoleIf.h"

//! Overlay for a Z story header
struct ZHeader
{
   uint8_t    version;             // $00: Version number
   uint8_t    flags1;              // $01: Flags 1
   STB::Big16 release;             // $02
   STB::Big16 himem;               // $04: Byte address of high memory
   STB::Big16 init_pc;             // $06: Byte address of first instruction
   STB::Big16 dict;                // $08: Byte address of dictionary
   STB::Big16 obj;                 // $0A: Byte address of object table
   STB::Big16 glob;                // $0C: Byte address of globals
   STB::Big16 stat;                // $0E: Byte address of static memory
   uint8_t    flags2;              // $10: Flags 2
   uint8_t    pad1[7];
   STB::Big16 abbr;                // $18: Byte address of abbreviations table
   STB::Big16 length;              // $1A: Length of file
   STB::Big16 checksum;            // $1C: Checksum

   uint8_t    interpreter_number;  // $1E:
   uint8_t    interpreter_version; // $1F:
   uint8_t    screen_lines;        // $20:
   uint8_t    screen_cols;         // $21:
   STB::Big16 screen_width;
   STB::Big16 screen_height;

   // TODO these are swapped v5 V v6?
   uint8_t font_height;
   uint8_t font_width;

   STB::Big16 routines;
   STB::Big16 static_strings;
   uint8_t    background_colour;
   uint8_t    foreground_colour;
   STB::Big16 terminating_characters;
   STB::Big16 width_text_stream3;
   STB::Big16 standard_revision;
   STB::Big16 alphabet_table;
   STB::Big16 header_ext;
   uint8_t    pad2[8];

   ZHeader() { assert(sizeof(ZHeader) == 64); }

   //! Check for supported versions
   bool isVersionValid() const { return (version >= 1) && (version <= 8); }

   //! Return size of story (bytes)
   uint32_t getStorySize() const
   {
      switch(version)
      {
      case 1:
      case 2:
      case 3:
         return length<<1;

      case 4:
      case 5:
         return length<<2;

      case 6:
      case 7:
      case 8:
         return length<<3;

      default:
         return 0;
      }
   }

   //! Return memory size limit (bytes)
   uint32_t getMemoryLimit() const
   {
      switch(version)
      {
      case 1:
      case 2:
      case 3:
         return 128 * 1024;

      case 4:
      case 5:
         return 256 * 1024;

      case 7:
         return 320 * 1024;

      case 6:
      case 8:
         return 512 * 1024;

      default:
         return 0;
      }
   }

   //! Convert a 16-but oacked address to a 32-bit address
   uint32_t unpackAddr(uint16_t packed_address, bool routine) const
   {
      switch(version)
      {
      case 1:
      case 2:
      case 3:
         return packed_address<<1;

      case 4:
      case 5:
         return packed_address<<2;

      case 6:
      case 7:
         return (packed_address<<2) + (routine ? routines<<3
                                               : static_strings<<3);

      case 8:
         return packed_address<<3;

      default:
         return 0;
      }
   }

   uint32_t getEntryPoint() const
   {
      return version == 6 ? unpackAddr(init_pc, /* routine */ true) + 1
                          : uint32_t(init_pc);
   }

   //!
   void print() const
   {
      printf("Version : %d\n", version);
      printf("Flags1  : %02X\n", flags1);
      printf("Flags2  : %02X\n", flags2);
      printf("Length  : 0x%04X\n", (uint16_t)length);
      printf("\n");
      printf("Dynamic Memory : 0x0040-0x%04X\n", (uint16_t)stat - 1);
      printf("Static Memory  : 0x%04X-0x%04X\n", (uint16_t)stat,
                                                  getStorySize() > 0xFFFF ? 0xFFFF
                                                                          : getStorySize());
      printf("High Memory    : 0x%04X-0x%05X\n", (uint16_t)himem, getStorySize());
      printf("\n");
      printf("Objects : 0x%04X\n", (uint16_t)obj);
      printf("Globals : 0x%04X\n", (uint16_t)glob);
      printf("Abbr    : 0x%04X\n", (uint16_t)abbr);
      printf("Dict    : 0x%04X\n", (uint16_t)dict);
      printf("Init PC : 0x%04X\n", (uint16_t)init_pc);
      printf("\n");
   }

   //!
   void init(ConsoleIf& console, ZConfig& config)
   {
      if(version <= 3)
      {
         if(config.status_line)      flags1 |= 1 << 4;
         if(config.screen_splitting) flags1 |= 1 << 5;
         if(config.var_pitch_font)   flags1 |= 1 << 6;
      }
      else
      {
         if(console.getAttr(ConsoleIf::BOLD))         flags1 |= 1 << 2;
         if(console.getAttr(ConsoleIf::ITALIC))       flags1 |= 1 << 3;
         if(console.getAttr(ConsoleIf::FIXED_FONT))   flags1 |= 1 << 4;
         if(console.getAttr(ConsoleIf::READ_TIMEOUT)) flags1 |= 1 << 7;

         if(version >= 5)
         {
            if(console.getAttr(ConsoleIf::COLOURS)) flags1 |= 1 << 0;

            if(version >= 6)
            {
               if(config.pictures) flags1 |= 1 << 1;
               if(config.sounds)   flags1 |= 1 << 5;
            }
         }
      }

      // 8.1.5.1
      if((version == 5) && !console.getAttr(ConsoleIf::GRAPHIC_FONT))
      {
         flags2 &= ~(1 << 3);
      }

      standard_revision = (config.interp_major_version << 8) | config.interp_minor_version;

      if(version >= 4)
      {
         interpreter_number  = 0;
         interpreter_version = 'A';
      }
      else
      {
         interpreter_number  = 0;
         interpreter_version = 0;
      }

      screen_lines = console.getAttr(ConsoleIf::LINES);
      screen_cols  = console.getAttr(ConsoleIf::COLS);

      font_height = console.getAttr(ConsoleIf::FONT_HEIGHT);
      font_width  = console.getAttr(ConsoleIf::FONT_WIDTH);
   }
};

#endif