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

#include "STB/IFF.h"

class ZBlorb
{
private:
   //! Resource index entry
   struct RIdx
   {
      struct Entry
      {
         STB::IFF::Ident  type;
         STB::IFF::UInt32 index;
         STB::IFF::UInt32 offset;
      };

      STB::IFF::UInt32 num_entries;
      Entry            entry[0];
   };

   bool findResource(const char*      filename,
                     const char*      resource_type,
                     unsigned         index,
                     STB::IFF::Ident& type,
                     uint32_t&        offset)
   {
      STB::IFF::Document doc;

      doc.read(filename);

      if (doc.isDocType("FORM") && doc.isFileType("IFRS"))
      {
         const RIdx* ridx = doc.load<RIdx>("RIdx");
         if (ridx == nullptr) return false;

         for(uint32_t i = 0; i<ridx->num_entries; i++)
         {
            if((ridx->entry[i].type == resource_type) &&
               (ridx->entry[i].index == index))
            {
               STB::IFF::Chunk* ch = doc.findChunk(ridx->entry[i].offset);
               if (ch != nullptr)
               {
                  type   = ch->getType();
                  offset = ridx->entry[i].offset + 8;
                  return true;
               }
            }
         }
      }

      return false;
   }

public:
   ZBlorb() = default;

   //! Find an Exec chunk of the given type
   bool findExecChunk(const char* filename, const char* type, uint32_t& offset)
   {
      STB::IFF::Ident ident;
      return findResource(filename, "Exec", 0, ident, offset) && (ident == type);
   }

   //! Find a Pict chunk for the given index
   bool findPictChunk(const char* filename,
                      uint32_t    index,
                      uint32_t&   offset,
                      bool&       is_png_not_jpeg)
   {
      STB::IFF::Ident ident;
      if(!findResource(filename, "Pict", 0, ident, offset)) return false;

      if(ident == "PNG ")
      {
         is_png_not_jpeg = true;
         return true;
      }
      else if(ident == "JPEG")
      {
         is_png_not_jpeg = false;
         return true;
      }

      return false;
   }

   //! Find a Snd chunk of the given type
   bool findSndChunk(const char* filename, uint32_t index, uint32_t& offset)
   {
      STB::IFF::Ident ident;
      return findResource(filename, "Snd ", 0, ident, offset);
   }
};

#endif
