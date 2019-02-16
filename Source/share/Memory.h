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

   //! Get address of first writable memory location
   Address getWriteStart() const { return write_start; }

   //! Get address of memory location after the last writable
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
      code_start    = start;
      code_end_incl = end_incl;
   }

   void limitWrite(Address start, Address end_incl)
   {
      write_start    = start;
      write_end_incl = end_incl;
   }

   //! Read byte from memory
   uint8_t readByte(Address addr) const
   {
      assert(addr < size());
      return raw[addr];
   }

   //! Read 16-bit word from memory
   uint16_t readWord(Address addr) const
   {
      uint16_t msb = readByte(addr);
      return (msb << 8) | readByte(addr + 1);
   }

   //! Read byte from code memory
   uint8_t codeByte(Address addr) const
   {
      assert((addr >= code_start) && (addr <= code_end_incl));
      return readByte(addr);
   }

   //! Read 16-bit word from code memory
   uint16_t codeWord(Address addr) const
   {
      uint16_t msb = codeByte(addr);
      return (msb << 8) | codeByte(addr + 1);
   }

   //! Set byte in any part of memory
   void setByte(Address addr, uint8_t byte)
   {
      assert(addr < size());
      raw[addr] = byte;
   }

   //! Write byte to writable memory
   void writeByte(Address addr, uint8_t byte)
   {
      assert((addr >= write_start) && (addr <= write_end_incl));
      setByte(addr, byte);
   }

   //! Write 16-bit word to writable memory
   void writeWord(Address addr, uint16_t word)
   {
      writeByte(addr,     word >> 8);
      writeByte(addr + 1, word & 0xFF);
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
