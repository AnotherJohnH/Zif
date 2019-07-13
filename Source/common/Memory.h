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

#ifndef IF_MEMORY_H
#define IF_MEMORY_H

#include <cassert>
#include <cstdint>
#include <vector>

namespace IF {

//! Memory implementation for an interactive fiction VM
class Memory
{
public:
   using Address = uint32_t;

   Memory() = default;

   //! Get memory size (bytes)
   size_t size() const { return raw.size(); }

   //! Get pointer to raw memory
   uint8_t* data() { return raw.data(); }

   //! Get read-only pointer to raw memory
   const uint8_t* data() const { return raw.data(); }

   //! Get address of first writable byte
   Address getWriteStart() const { return write_start; }

   //! Get address of last writable byte
   Address getWriteEnd() const { return write_end_incl; }

   //! Set memory size (bytes)
   void resize(size_t size)
   {
      raw.resize(size);

      limitCode(0, size - 1);
      limitWrite(0, size - 1);
   }
 
   void limitCode(Address start, Address end_incl)
   {
      if ((end_incl >= size()) || (start > end_incl)) throw "memory map fault";
      code_start    = start;
      code_end_incl = end_incl;
   }

   void limitWrite(Address start, Address end_incl)
   {
      if ((end_incl >= size()) || (start > end_incl)) throw "memory map fault";
      write_start    = start;
      write_end_incl = end_incl;
   }

   //! Read byte from memory
   uint8_t read8(Address addr) const
   {
      if (addr >= size()) throw "memory read fault";
      return raw[addr];
   }

   //! Read 16-bit word from memory
   uint16_t read16(Address addr) const
   {
      if (addr >= (size() - 1)) throw "memory read fault";
      return (uint16_t(raw[addr]) << 8) |
                       raw[addr + 1];
   }

   //! Read 24-bit word from memory
   uint32_t read24(Address addr) const
   {
      if (addr >= (size() - 2)) throw "memory read fault";
      return (uint32_t(raw[addr    ]) << 16) |
             (uint32_t(raw[addr + 1]) <<  8) |
                       raw[addr + 2];
   }

   //! Read 32-bit word from memory
   uint32_t read32(Address addr) const
   {
      if (addr >= (size() - 3)) throw "memory read fault";
      return (uint32_t(raw[addr    ]) << 24) |
             (uint32_t(raw[addr + 1]) << 16) |
             (uint32_t(raw[addr + 2]) <<  8) |
                       raw[addr + 3];
   }

   //! Read data from memory
   template <typename TYPE>
   TYPE read(Address addr)
   {
      switch(sizeof(TYPE))
      {
      case 1: return read8(addr);
      case 2: return read16(addr);
      case 4: return read32(addr);

      default:
         assert(!"unsupported data size");
         break;
      }
   }

   //! Fetch byte from code memory
   uint8_t fetch8(Address addr) const
   {
      if((addr < code_start) || (addr > code_end_incl)) throw "memory fetch fault";
      return raw[addr];
   }

   //! Fetch 16-bit word from code memory
   uint16_t fetch16(Address addr) const
   {
      if((addr < code_start) || (addr > (code_end_incl - 1))) throw "memory fetch fault";
      return (uint16_t(raw[addr]) << 8) |
                       raw[addr + 1];
   }

   //! Fetch 24-bit word from code memory
   uint16_t fetch24(Address addr) const
   {
      if((addr < code_start) || (addr > (code_end_incl - 2))) throw "memory fetch fault";
      return (uint32_t(raw[addr    ]) << 16) |
             (uint32_t(raw[addr + 1]) <<  8) |
                       raw[addr + 2];
   }

   //! Fetch 32-bit word from code memory
   uint32_t fetch32(Address addr) const
   {
      if((addr < code_start) || (addr > (code_end_incl - 3))) throw "memory fetch fault";
      return (uint32_t(raw[addr    ]) << 24) |
             (uint32_t(raw[addr + 1]) << 16) |
             (uint32_t(raw[addr + 2]) <<  8) |
                       raw[addr + 3];
   }

   //! Set byte in any part of memory
   void set8(Address addr, uint8_t byte)
   {
      if (addr >= size()) throw "memory set fault";
      raw[addr] = byte;
   }

   //! Write byte to writable memory
   void write8(Address addr, uint8_t byte)
   {
      if((addr < write_start) || (addr > write_end_incl)) throw "memory write fault";
      raw[addr] = byte;
   }

   //! Write 16-bit word to writable memory
   void write16(Address addr, uint16_t word)
   {
      if((addr < write_start) || (addr > (write_end_incl - 1))) throw "memory write fault";
      raw[addr    ] = word >> 8;
      raw[addr + 1] = uint8_t(word);
   }

   //! Write 24-bit word to writable memory
   void write24(Address addr, uint32_t word)
   {
      if((addr < write_start) || (addr > (write_end_incl - 2))) throw "memory write fault";
      raw[addr    ] = uint8_t(word >> 16);
      raw[addr + 1] = uint8_t(word >>  8);
      raw[addr + 2] = uint8_t(word);
   }

   //! Write 32-bit word to writable memory
   void write32(Address addr, uint32_t word)
   {
      if((addr < write_start) || (addr > (write_end_incl - 3))) throw "memory write fault";
      raw[addr    ] = uint8_t(word >> 24);
      raw[addr + 1] = uint8_t(word >> 16);
      raw[addr + 2] = uint8_t(word >>  8);
      raw[addr + 3] = uint8_t(word);
   }

   //! Write data to memory
   template <typename TYPE>
   void write(Address addr, TYPE data)
   {
      switch(sizeof(TYPE))
      {
      case 1: write8(addr, data); break;
      case 2: write16(addr, data); break;
      case 4: write32(addr, data); break;

      default:
         assert(!"unsupported data size");
         break;
      }
   }

protected:
   Address              code_start{0};
   Address              code_end_incl{0};
   Address              write_start{0};
   Address              write_end_incl{0};
   std::vector<uint8_t> raw;
};

} // namespace IF

#endif
