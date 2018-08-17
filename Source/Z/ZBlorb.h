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
#include <cstring>

#include "PLT/File.h"
#include "STB/Endian.h"

class ZBlorb
{
private:
   //! IFF 4-char ident
   struct Ident
   {
      char value[4];

      bool is(const char* expected) const
      {
         return strncmp(value, expected, sizeof(value)) == 0;
      }
   };

   //! IFF chunk header
   struct ChunkHeader
   {
      Ident      ident;
      STB::Big32 size;

      bool read(PLT::File& file)
      {
         return file.read(this, sizeof(ChunkHeader));
      }

      bool match(PLT::File& file, const char* expected_id)
      {
         if (!read(file)) return false;
         return ident.is(expected_id);
      }
   };

   //! Resource index entry
   struct RIdxEntry
   {
      Ident      type;
      STB::Big32 index;
      STB::Big32 offset;

      bool read(PLT::File& file)
      {
         return file.read(this, sizeof(RIdxEntry));
      }
   };

   //! Find a chunk of the given type a chunk
   bool findResource(const char*  filename,
                     const char*  resource_type,
                     unsigned     index,
                     ChunkHeader& header,
                     uint32_t&    offset)
   {
      PLT::File file(nullptr, filename);
      if(!file.openForRead()) return false;

      // Confirm IFF file format
      if (!header.match(file, "FORM")) return false;

      // Confirm FORM type is IFRS
      Ident type;
      if (!file.read(&type, sizeof(type))) return false;
      if (!type.is("IFRS")) return false;

      // Confirm first chunk type is a resource index
      if (!header.match(file, "RIdx")) return false;

      STB::Big32 num_entries;
      if (!file.read(&num_entries, 4)) return false;

      for(uint32_t i=0; i<num_entries; i++)
      {
         RIdxEntry entry;
         if (!entry.read(file)) return false;

         if (entry.type.is(resource_type) && (entry.index == index))
         {
            file.seek(entry.offset);
            if (!header.read(file)) return false;
            offset = entry.offset + sizeof(ChunkHeader);
            return true;
         }
      }

      return false;
   }

public:
   ZBlorb() = default;

   //! Find an Exec chunk of the given type
   bool findExecChunk(const char* filename, const char* type, uint32_t& offset)
   {
      ChunkHeader header;
      if (!findResource(filename, "Exec", 0, header, offset)) return false;

      return header.ident.is(type);
   }

   //! Find a Pict chunk for the given index
   bool findPictChunk(const char* filename,
                      uint32_t    index,
                      uint32_t&   offset,
                      bool&       is_png_not_jpeg)
   {
      ChunkHeader header;
      if (!findResource(filename, "Pict", 0, header, offset)) return false;

           if (header.ident.is("PNG ")) { is_png_not_jpeg = true;  return true; }
      else if (header.ident.is("JPEG")) { is_png_not_jpeg = false; return true; }

      return false;
   }

   //! Find a Snd chunk of the given type
   bool findSndChunk(const char* filename, uint32_t index, uint32_t& offset)
   {
      ChunkHeader header;
      if (!findResource(filename, "Snd ", 0, header, offset)) return false;

      return true;
   }
};

#endif
