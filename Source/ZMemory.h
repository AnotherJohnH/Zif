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

#include "PLT/File.h"


//! Memory for ZMachine
class ZMemory
{
private:
   static const uint32_t MAX_SIZE{512 * 1024};

   uint32_t limit{0};
   uint8_t  data[MAX_SIZE];

public:
   ZMemory() = default;

   //! Set memory size limit (bytes)
   void resize(uint32_t limit_)
   {
      assert((limit_ > limit) && (limit_ <= MAX_SIZE));

      uint32_t prev_limit = limit;
      limit = limit_;

      zero(prev_limit, limit);
   }

   //! Get constant reference to a memory byte
   const uint8_t& operator[](uint32_t addr) const
   {
      assert(addr < limit);

      return data[addr];
   }

   //! Get reference to a memory byte
   uint8_t& operator[](uint32_t addr)
   {
      assert(addr < limit);

      return data[addr];
   }

   // 16-bit word access

   //! Read 16-bit word
   uint16_t readWord(uint32_t addr) const
   {
      assert(addr < (limit - 1));

      uint16_t msb = data[addr];
      return (msb << 8) | data[addr + 1];
   }

   //! Write 16-bit word
   void writeWord(uint32_t addr, uint16_t word)
   {
      assert(addr < (limit - 1));

      data[addr]     = word >> 8;
      data[addr + 1] = word & 0xFF;
   }

   //! Compute checksum for a block of memory
   uint16_t checksum(uint32_t start, uint32_t end)
   {
      assert((start < limit) && (end <= limit));

      uint16_t checksum = 0;

      for(uint32_t addr = start; addr < end; ++addr)
      {
         checksum += data[addr];
      }

      return checksum;
   }

   //! Save a block of memory to an open file stream
   bool save(PLT::File& file, uint32_t start, uint32_t end) const
   {
      assert((start < limit) && (end <= limit));

      return file.write(&data[start], end - start);
   }

   //! Load a block of memory from an open file stream
   bool load(PLT::File& file, uint32_t start, uint32_t end)
   {
      assert(start < end);

      if (end > limit)
      {
         resize(end);
      }

      assert((start < limit) && (end <= limit));

      return file.read(&data[start], end - start);
   }

   //! Zero a block of memory
   void zero(uint32_t start, uint32_t end)
   {
      assert((start < limit) && (end <= limit));

      for(uint32_t addr = start; addr < end; addr++)
      {
         data[addr] = 0;
      }
   }

   //! Copy a block of memory (copy lowest address first)
   void copyForward(uint32_t from, uint32_t to, uint32_t size)
   {
      assert(((from + size) <= limit) && ((to + size) <= limit));

      for(uint32_t i = 0; i < size; i++)
      {
         data[to + i] = data[from + i];
      }
   }

   //! Copy a block of memory (copy highest address first)
   void copyBackward(uint32_t from, uint32_t to, uint32_t size)
   {
      assert(((from + size) <= limit) && ((to + size) <= limit));

      for(uint32_t i = size; i > 0; i--)
      {
         data[to + i - 1] = data[from + i - 1];
      }
   }
};

#endif
