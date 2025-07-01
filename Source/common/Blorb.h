//-------------------------------------------------------------------------------
// Copyright (c) 2018 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

