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

   uint32_t limit{MAX_SIZE};
   uint8_t  data[MAX_SIZE];

public:
   ZMemory()
   {
      clear(0, MAX_SIZE);
   }

   //! Set memory size limit (bytes)
   void resize(uint32_t limit_)
   {
      assert(limit_ <= MAX_SIZE);
      limit = limit_;
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
      uint16_t msb = operator[](addr);
      return (msb << 8) | operator[](addr + 1);
   }

   //! Write 16-bit word
   void writeWord(uint32_t addr, uint16_t word)
   {
      operator[](addr)     = word >> 8;
      operator[](addr + 1) = word & 0xFF;
   }

   //! Compute checksum for a block of memory
   uint16_t checksum(uint32_t start, uint32_t end)
   {
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
      assert((start < MAX_SIZE) && (end <= MAX_SIZE));

      return file.read(&data[start], end - start);
   }

   //! Clear a block of memory
   void clear(uint32_t start, uint32_t end)
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
      for(uint32_t i = 0; i < size; i++)
      {
         operator[](to + i) = operator[](from + i);
      }
   }

   //! Copy a block of memory (copy highest address first)
   void copyBackward(uint32_t from, uint32_t to, uint32_t size)
   {
      for(uint32_t i = size; i > 0; i--)
      {
         operator[](to + i - 1) = operator[](from + i - 1);
      }
   }
};

#endif
