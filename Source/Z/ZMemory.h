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
   Address getStaticAddr() const { return static_mem; }

   //! Get start of hi-memory
   Address getHimemAddr() const { return hi_mem; }

   //! Get size of memory
   Address getSize() const { return size; }

   //! Configure memory for a game
   bool configure(const ZHeader* header)
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

      zero(sizeof(ZHeader), size);

      return true;
   }
 
   //! Get constant reference to a memory byte
   const uint8_t& operator[](uint32_t addr) const
   {
      assert(addr < size);

      return data[addr];
   }

   //! Get reference to a memory byte
   uint8_t& operator[](uint32_t addr)
   {
      // TODO assert(addr < static_mem);
      assert(addr < size);

      return data[addr];
   }

   // 16-bit word access

   //! Read 16-bit word
   uint16_t readWord(uint32_t addr) const
   {
      assert(addr < (size - 1));

      uint16_t msb = data[addr];
      return (msb << 8) | data[addr + 1];
   }

   //! Write 16-bit word
   void writeWord(uint32_t addr, uint16_t word)
   {
      assert(addr < (static_mem - 1));

      data[addr]     = word >> 8;
      data[addr + 1] = word & 0xFF;
   }

   //! Compute checksum for a block of memory
   uint16_t checksum(uint32_t start, uint32_t end)
   {
      assert((start < size) && (end <= size));

      uint16_t checksum = 0;

      for(uint32_t addr = start; addr < end; ++addr)
      {
         checksum += data[addr];
      }

      return checksum;
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
