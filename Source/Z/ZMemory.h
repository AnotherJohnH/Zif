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

#include "ZHeader.h"

//! Z machine memory
class ZMemory
{
public:
   using Address = uint32_t;

   ZMemory() = default;

   //! Get start of static memory
   Address getStaticStart() const { return static_mem; }

   //! Get start of hi-memory
   Address getHimemStart() const { return hi_mem; }

   //! Get size of memory
   Address getSize() const { return size; }

   //! Get read-only pointer to raw memory
   const uint8_t* getData() const { return data; }

   //! Get writable pointer to header
   ZHeader* getHeader() { return reinterpret_cast<ZHeader*>(data); }

   //! Configure memory for a game
   bool init(const ZHeader* header)
   {
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

      static_mem  = header->stat;
      hi_mem      = header->himem;
      size        = header->getStorySize();

      memcpy(data, header, sizeof(ZHeader));

      return true;
   }

   //! Reset memory from game image
   void reset(const uint8_t* image)
   {
      memcpy(data + sizeof(ZHeader),
            image + sizeof(ZHeader),
            getSize() - sizeof(ZHeader));
   }
 
   //! Read byte from any part of memory
   uint8_t get(Address addr) const
   {
      assert(addr < size);

      return data[addr];
   }

   //! Read 16-bit word from any part of memory
   uint16_t getWord(Address addr) const
   {
      assert(addr < (size - 1));

      uint16_t msb = data[addr];
      return (msb << 8) | data[addr + 1];
   }

   //! Write byte to any part of memory
   void set(Address addr, uint8_t byte)
   {
      assert(addr < size);

      data[addr] = byte;
   }

   //! Read byte from dynamic or static memory
   uint8_t read(Address addr) const
   {
      // TODO assert(in static memory)
      assert(addr < size);

      return data[addr];
   }

   //! Write byte to dynamic memory
   void write(Address addr, uint8_t byte)
   {
      assert(addr < static_mem);

      data[addr] = byte;
   }

   //! Read 16-bit word from dynamic or static memory
   uint16_t readWord(Address addr) const
   {
      // TODO assert(in static memory)
      assert(addr < (size - 1));

      uint16_t msb = data[addr];
      return (msb << 8) | data[addr + 1];
   }

   //! Write 16-bit word to dynamic memory
   void writeWord(Address addr, uint16_t word)
   {
      assert(addr < (static_mem - 1));

      data[addr]     = word >> 8;
      data[addr + 1] = word & 0xFF;
   }

   //! Zero a block of memory
   void zero(uint32_t start, uint32_t end)
   {
      assert((start < size) && (end <= size));

      for(uint32_t addr = start; addr < end; addr++)
      {
         data[addr] = 0;
      }
   }

   //! Copy a block of memory (copy lowest address first)
   void copyForward(uint32_t from, uint32_t to, uint32_t n)
   {
      assert(((from + n) <= size) && ((to + n) <= static_mem));

      for(uint32_t i = 0; i < n; i++)
      {
         data[to + i] = data[from + i];
      }
   }

   //! Copy a block of memory (copy highest address first)
   void copyBackward(uint32_t from, uint32_t to, uint32_t n)
   {
      assert(((from + n) <= size) && ((to + n) <= static_mem));

      for(uint32_t i = n; i > 0; i--)
      {
         data[to + i - 1] = data[from + i - 1];
      }
   }

private:
   static const Address MAX_SIZE{512 * 1024};

   Address  static_mem{0};
   Address  hi_mem{0};
   Address  size{0};
   uint8_t  data[MAX_SIZE];
};

#endif
