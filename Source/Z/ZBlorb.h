//------------------------------------------------------------------------------
// Copyright (c) 2018 John D. Haughton
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

#ifndef ZBLORB_H
#define ZBLORB_H

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "PLT/File.h"
#include "STB/Endian.h"

class ZBlorb
{
private:
   struct ChunkHeader
   {
      char       id[4];
      STB::Big32 size;

      bool match(PLT::File& file, const char* expected_id)
      {
         if (!file.read(this, sizeof(ChunkHeader))) return false;
         return strncmp(id, expected_id, 4) == 0;
      }
   };

   struct RIdxEntry
   {
      char       id[4];
      STB::Big32 index;
      STB::Big32 offset;
   };

public:
   ZBlorb() = default;

   bool init(const char* filename)
   {
      PLT::File file(nullptr, filename);
      if(!file.openForRead()) return false;

      ChunkHeader header;

      // Confirm IFF file format
      if (!header.match(file, "FORM")) return false;

      // Confirm FORM type is IFRS
      char type[4];
      if (!file.read(type, 4)) return false;
      if (strncmp(type, "IFRS", 4) != 0) return false;

      // Confirm first chunk type is a resource index
      if (!header.match(file, "RIdx")) return false;

      STB::Big32 num_entries;
      if (!file.read(&num_entries, 4)) return false;

      for(uint32_t i=0; i<num_entries; i++)
      {
         RIdxEntry entry;
         if (!file.read(&entry, sizeof(entry))) return false;

              if (strncmp(entry.id, "Exec", 4) == 0) { exec = entry.offset; }
         else if (strncmp(entry.id, "Pict", 4) == 0) { pict = entry.offset; }
         else if (strncmp(entry.id, "Snd ", 4) == 0) { snd  = entry.offset; }
      }

      if (exec == 0) return false;

      file.seek(exec);
      if (!header.match(file, "ZCOD")) return false;
      exec += sizeof(ChunkHeader);

      return true;
   }

   uint32_t execOffset()
   {
       return exec;
   }

private:
   uint32_t exec{0};
   uint32_t pict{0};
   uint32_t snd{0};
};

#endif
