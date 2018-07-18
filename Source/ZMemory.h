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
   ZMemory() { clear(0, MAX_SIZE); }

   //! Set memory size limit (bytes)
   void setLimit(uint32_t limit_)
   {
      assert(limit_ <= MAX_SIZE);
      limit = limit_;
   }

   // Byte access

   //! Read byte
   const uint8_t& readByte(uint32_t addr) const
   {
      assert(addr < limit);
      return data[addr];
   }

   //! Read byte and increment address
   uint8_t fetchByte(uint32_t& addr) const
   {
      assert(addr < limit);
      return data[addr++];
   }

   //! Write byte
   void writeByte(uint32_t addr, uint8_t value)
   {
      assert(addr < limit);
      data[addr] = value;
   }

   // 16-bit word access

   //! Read 16-bit word
   uint16_t readWord(uint32_t addr) const
   {
      uint16_t word = readByte(addr);
      return (word << 8) | readByte(addr + 1);
   }

   //! Read 16-bit word and increment address
   uint16_t fetchWord(uint32_t& addr) const
   {
      uint16_t value = fetchByte(addr);
      return (value << 8) | fetchByte(addr);
   }

   //! Write 16-bit word
   void writeWord(uint32_t addr, uint16_t value)
   {
      writeByte(addr, value >> 8);
      writeByte(addr + 1, value & 0xFF);
   }


   //! Load a block of memory from an open file stream.
   //! With optional checksum calculation
   bool load(PLT::File& file, uint32_t start, uint32_t end, uint16_t* checksum_ptr = nullptr)
   {
      assert((start < MAX_SIZE) && (end <= MAX_SIZE));

      if(!file.read(&data[start], end - start))
      {
         return false;
      }

      if(checksum_ptr != nullptr)
      {
         uint16_t checksum = 0;

         for(uint32_t addr = start; addr < end; ++addr)
         {
            checksum += data[addr];
         }

         *checksum_ptr = checksum;
      }

      return true;
   }

   //! Save a block of memory to an open file stream
   bool save(PLT::File& file, uint32_t start, uint32_t end) const
   {
      assert((start < MAX_SIZE) && (end <= MAX_SIZE));

      return file.write(&data[start], end - start);
   }

   //! Clear a block of memory
   void clear(uint32_t start, uint32_t end)
   {
      for(uint32_t addr = start; addr < end; addr++)
      {
         writeByte(addr, 0);
      }
   }

   //! Copy a block of memory (copy lowest address first)
   void copyForward(uint32_t from, uint32_t to, uint32_t size)
   {
      for(uint32_t i = 0; i < size; i++)
      {
         writeByte(to + i, readByte(from + i));
      }
   }

   //! Copy a block of memory (copy highest address first)
   void copyBackward(uint32_t from, uint32_t to, uint32_t size)
   {
      for(uint32_t i = size; i > 0; i--)
      {
         writeByte(to + i - 1, readByte(from + i - 1));
      }
   }
};

#endif
