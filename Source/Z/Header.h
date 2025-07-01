//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cassert>
#include <cstdint>

#include "STB/Endian.h"

#include "common/Console.h"

#include "Z/Config.h"

namespace Z {

//! Overlay for a Z story header
struct Header
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
   uint8_t    pad1;
   uint8_t    serial[6];           // $12: Serial number
   STB::Big16 abbr;                // $18: Byte address of abbreviations table
   STB::Big16 length;              // $1A: Length of file
   STB::Big16 checksum;            // $1C: Checksum

   uint8_t    interpreter_number;  // $1E:
   uint8_t    interpreter_version; // $1F:
   uint8_t    screen_lines;        // $20:
   uint8_t    screen_cols;         // $21:
   STB::Big16 screen_width;        // $22
   STB::Big16 screen_height;       // $24

private:
   uint8_t font_height;               // $26
   uint8_t font_width;                // $27

public:
   STB::Big16 routines;               // $28
   STB::Big16 static_strings;         // $2A
   uint8_t    background_colour;      // $2C
   uint8_t    foreground_colour;      // $2D
   STB::Big16 terminating_characters; // $2E
   STB::Big16 width_text_stream3;     // $30
   STB::Big16 standard_revision;      // $32
   STB::Big16 alphabet_table;         // $34
   STB::Big16 header_ext;
   uint8_t    pad2[8];

   Header() { assert(sizeof(Header) == 64); }

   //! Check for supported versions
   bool isVersionValid() const { return (version >= 1) && (version <= 8); }

   unsigned getFontHeight() const { return version != 6 ? font_height : font_width; }

   unsigned getFontWidth() const { return version != 6 ? font_width : font_height; }

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

   void setStorySize(uint32_t size)
   {
      if ((version <= 3) && (size < 0x20000))
      {
         length = size >> 1;
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

   //! Reset header for this interpreter
   void reset(Console& console, Config& config)
   {
      // Flags 1
      if(version <= 3)
      {
         if(!config.status_line)     flags1 |= 1 << 4;
         if(config.screen_splitting) flags1 |= 1 << 5;
         if(config.var_pitch_font)   flags1 |= 1 << 6;
      }
      else
      {
         if(console.getAttr(Console::BOLD))         flags1 |= 1 << 2;
         if(console.getAttr(Console::ITALIC))       flags1 |= 1 << 3;
         if(console.getAttr(Console::FIXED_FONT))   flags1 |= 1 << 4;

         if(console.getAttr(Console::READ_TIMEOUT)) flags1 |= 1 << 7;

         if(version >= 5)
         {
            if(console.getAttr(Console::COLOURS)) flags1 |= 1 << 0;

            if(version >= 6)
            {
               if(config.pictures) flags1 |= 1 << 1;
               if(config.sounds)   flags1 |= 1 << 5;
            }
         }
      }

      // Flags 2 - 8.1.5.1
      if((version == 5) && !console.getAttr(Console::GRAPHIC_FONT))
      {
         if (!config.pictures) flags2 &= ~(1 << 3);
         if (!config.undo)     flags2 &= ~(1 << 4);
         if (!config.mouse)    flags2 &= ~(1 << 5);
         if (!config.sounds)   flags2 &= ~(1 << 7);
         if (!config.menus)    flags2 &= ~(1 << 8);
      }

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

      screen_lines  = console.getAttr(Console::LINES);
      screen_cols   = console.getAttr(Console::COLS);

      font_height = console.getAttr(Console::FONT_HEIGHT);
      font_width  = console.getAttr(Console::FONT_WIDTH);

      if(version >= 5)
      {
         screen_width  = screen_cols * font_width;
         screen_height = screen_lines * font_height;
      }

      background_colour = 2;
      foreground_colour = 9;

      standard_revision = (config.interp_major_version << 8) | config.interp_minor_version;
   }
};

} // namespace Z

