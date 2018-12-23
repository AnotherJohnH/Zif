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

#ifndef ZMEMORY_H
#define ZMEMORY_H

#include <cassert>
#include <cstdint>
#include <cstring>

#include "ZStory.h"

//! Z machine memory
class ZMemory
{
public:
   using Address = uint32_t;

   ZMemory() = default;

   //! Get start of static memory
   Address getStaticStart() const { return static_mem; }

   //! Get size of memory
   Address getSize() const { return size; }

   //! Get read-only pointer to raw memory
   const uint8_t* getData() const { return data; }

   //! Get writable pointer to header
   ZHeader* getHeader() { return reinterpret_cast<ZHeader*>(data); }

   //! Configure memory for a game
   bool init(const ZStory& story_)
   {
      const ZHeader* header = story_.getHeader();

      // Validate memory limit
      if (header->getMemoryLimit() > MAX_SIZE) return false;

      // Validate story size
      if (header->getStorySize() > header->getMemoryLimit()) return false;

      // Validate static memory region
      if ((header->stat < sizeof(ZHeader)) ||
          (header->stat > 0xFFFF) ||
          (header->stat > header->getStorySize()))
      {
          return false;
      }

      // Validate hi-memory region
      if ((header->himem < sizeof(ZHeader)) ||
          (header->himem > header->getStorySize()) ||
          (header->himem < header->stat))
      {
          return false;
      }

      story          = &story_;
      static_mem     = header->stat;
      static_mem_end = header->getStorySize() < 0x10000 ? header->getStorySize() : 0x10000;
      size           = header->getStorySize();

      memcpy(data, header, sizeof(ZHeader));

      return true;
   }

   //! Reset memory from game image
   void reset()
   {
      // TODO the header should be reset (only bits 0 and 1 from Flags 2
      //      shoud be preserved)

      memcpy(data + sizeof(ZHeader),
             story->data() + sizeof(ZHeader),
             story->size() - sizeof(ZHeader));
   }
 
   //! Read byte from any part of memory
   uint8_t getByte(Address addr) const
   {
      assert(addr < size);

      return data[addr];
   }

   //! Write byte to any part of memory
   void setByte(Address addr, uint8_t byte)
   {
      assert(addr < size);

      data[addr] = byte;
   }

   //! Read code byte
   uint8_t codeByte(Address addr) const
   {
      assert(addr < size);

      return data[addr];
   }

   //! Read 16-bit code word
   uint16_t codeWord(Address addr) const
   {
      assert(addr < (size - 1));

      uint16_t msb = data[addr];
      return (msb << 8) | data[addr + 1];
   }

   //! Read byte from dynamic or static memory
   uint8_t readByte(Address addr) const
   {
      assert(addr < static_mem_end);

      return data[addr];
   }

   //! Read 16-bit word from dynamic or static memory
   uint16_t readWord(Address addr) const
   {
      assert(addr < (static_mem_end - 1));

      uint16_t msb = data[addr];
      return (msb << 8) | data[addr + 1];
   }

   //! Write byte to dynamic memory
   void writeByte(Address addr, uint8_t byte)
   {
      assert(addr < static_mem);

      data[addr] = byte;
   }

   //! Write 16-bit word to dynamic memory
   void writeWord(Address addr, uint16_t word)
   {
      assert(addr < (static_mem - 1));

      data[addr]     = word >> 8;
      data[addr + 1] = word & 0xFF;
   }

private:
   static const Address MAX_SIZE{512 * 1024};

   const ZStory* story{nullptr};
   Address       static_mem{0};
   Address       static_mem_end{0};
   Address       size{0};
   uint8_t       data[MAX_SIZE];
};

#endif
