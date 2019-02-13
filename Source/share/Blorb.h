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

#ifndef BLORB_H
#define BLORB_H

#include "STB/IFF.h"

class Blorb
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

public:
   enum class Resource
   {
      EXEC,
      PICT,
      SND
   };

   Blorb() = default;

   bool findResource(const std::string& filename,
                     Resource           resource,
                     unsigned           index,
                     std::string&       type,
                     uint32_t&          offset)
   {
      STB::IFF::Document doc;

      if (!doc.read(filename, "FORM", "IFRS"))
         return false;

      const RIdx* ridx = doc.load<RIdx>("RIdx");
      if (ridx == nullptr) return false;

      const char* resource_type{};

      switch(resource)
      {
      case Resource::EXEC: resource_type = "Exec"; break;
      case Resource::PICT: resource_type = "Pict"; break;
      case Resource::SND:  resource_type = "Snd "; break;
      }

      for(uint32_t i = 0; i<ridx->num_entries; i++)
      {
         if((ridx->entry[i].type == resource_type) &&
            (ridx->entry[i].index == index))
         {
            STB::IFF::Chunk* ch = doc.findChunk(ridx->entry[i].offset);
            if (ch != nullptr)
            {
               ch->getType().get(type);
               offset = ridx->entry[i].offset + 8;
               return true;
            }
         }
      }

      return false;
   }
};

#endif
