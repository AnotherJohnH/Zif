//------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
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

#ifndef LEVEL9_STORY_H
#define LEVEL9_STORY_H

#include "share/Story.h"

#include "Level9/Header.h"

namespace Level9 {

//! Manage Level9 story image
class Story : public IF::StoryBase<Level9::Header>
{
public:
   Story() = default;

   virtual bool checkHeader(FILE* fp) override
   {
      uint8_t magic[4];

      if (fread(&magic, 1, 4, fp) == 4)
      {
         return isMagic(magic);
      }

      return false;
   }

   virtual bool validateHeader(FILE* fp, size_t& file_size) override
   {
      const Header* header = getHeader();

      if (!isMagic(header->magic))
      {
         error = "Invalid Level9 magic key";
         return false;
      }

      file_size = 0;

      return true;
   }

   virtual bool validateImage() const override
   {
      return true;
   }

   virtual IF::Memory::Address getEntryPoint() const override { return 0; }

   //! Encode Quetzal header chunk
   virtual void encodeQuetzalHeader(STB::IFF::Document& doc, uint32_t /* pc */) const override
   {
      // TODO do the right thing here

      STB::IFF::Chunk* ifhd_chunk = doc.newChunk("IFhd", 4);

      ifhd_chunk->push(getHeader(), 4);
   }

   //! Decode Quetzal header chunk
   virtual bool decodeQuetzalHeader(STB::IFF::Document& doc, uint32_t& pc) const override
   {
       const void* ifhd = doc.load<void>("IFhd");
       if (ifhd == nullptr)
       {
          error = "IFhd chunk not found";
          return false;
       }

      // TODO do the right thing here

       if (memcmp(ifhd, getHeader(), 4) != 0)
       {
          error = "IFhd mismatch";
          return false;
       }

       pc = 0;

       return true;
   }

   //! Initialise VM memory for this Z-story image
   virtual void prepareMemory(IF::Memory& memory) const override
   {
   }

   //! Reset VM memory for this Z-story image
   virtual void resetMemory(IF::Memory& memory) const override
   {
   }

private:
   bool isMagic(const uint8_t* magic)
   {
      return (magic[0] == 0xFE) &&
             (magic[1] == 0x58) &&
             (magic[2] == 0x27) &&
             (magic[3] == 0x9B);
   }
};

} // namespace Level9

#endif
