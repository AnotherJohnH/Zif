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

#ifndef GLULX_STORY_H
#define GLULX_STORY_H

#include "share/Story.h"

#include "Glulx/Header.h"

namespace Glulx {

//! Manage Glulx story image
class Story : public IF::StoryBase<Glulx::Header>
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
         error = "Invalid Glulx magic key";
         return false;
      }

      file_size = header->ext_start;

      return true;
   }

   virtual bool validateImage() const override
   {
      const Header* header = getHeader();

      uint32_t checksum = 0;

      for(size_t i=0; i<header->ext_start; i+= 4)
      {
         const uint32_t& word = (const uint32_t&) image[i];

         // XXX assuming the host machine is little endian
         checksum += STB::endianSwap(word);
      }

      return header->checksum == checksum;
   }

   virtual IF::Memory::Address getEntryPoint() const override
   {
      return getHeader()->start_func;
   }

   //! Encode Quetzal header chunk
   virtual void encodeQuetzalHeader(STB::IFF::Document& doc, uint32_t /* pc */) const override
   {
      STB::IFF::Chunk* ifhd_chunk = doc.newChunk("IFhd", 128);

      ifhd_chunk->push(getHeader(), 128);
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

       if (memcmp(ifhd, getHeader(), 128) != 0)
       {
          error = "IFhd mismatch";
          return false;
       }

       // PC is (very sensibly) stored on the stack
       pc = 0;

       return true;
   }

   //! Initialise VM memory for this Z-story image
   virtual void prepareMemory(IF::Memory& memory) const override
   {
      const Header* header = getHeader();
      memory.resize(header->end_mem);
   }

   //! Reset VM memory for this Z-story image
   virtual void resetMemory(IF::Memory& memory) const override
   {
      const Header* header = getHeader();
      memcpy(memory.data(), data(), size());
      memset(memory.data() + header->ext_start, 0, header->end_mem - header->ext_start);
   }

private:
   bool isMagic(const uint8_t* magic)
   {
      return (magic[0] == 'G') &&
             (magic[1] == 'l') &&
             (magic[2] == 'u') &&
             (magic[3] == 'l');
   }
};

} // namespace Glulx

#endif
