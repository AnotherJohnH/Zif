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

#ifndef STACK_H
#define STACK_H

#include <cassert>
#include <cstdint>
#include <vector>

namespace {

class Stack
{
public:
   Stack(size_t max_size_)
      : max_size(max_size_)
   {
      raw.reserve(max_size);
   }

   //! Check if stack is empty
   bool empty() const { return raw.empty(); }

   //! Get memory size (bytes)
   size_t size() const { return raw.size(); }

   //! Pointer to raw stack contents
   const uint8_t* data() const { return raw.data(); }

   //! Read 8-bit value from an absolute offset into the stack
   uint8_t read8(size_t offset) const
   {
      if (offset >= size()) throw "stack fault";
      return raw[offset];
   }

   //! Read 16-bit value from an absolute offset into the stack
   uint16_t read16(size_t offset) const
   {
      if ((size() < 2) || (offset > (size() - 2))) throw "stack fault";
      return (raw[offset    ] << 8) |
             (raw[offset + 1]);
   }

   //! Read 24-bit value from an absolute offset into the stack
   uint32_t read24(size_t offset) const
   {
      if ((size() < 3) || (offset > (size() - 3))) throw "stack fault";
      return (raw[offset    ] << 16) | 
             (raw[offset + 1] <<  8) | 
             (raw[offset + 2]);
   }

   //! Read 32-bit value from an absolute offset into the stack
   uint32_t read32(size_t offset) const
   {
      if ((size() < 4) || (offset > (size() - 4))) throw "stack fault";
      return (raw[offset    ] << 24) | 
             (raw[offset + 1] << 16) | 
             (raw[offset + 2] <<  8) | 
             (raw[offset + 3]);
   }

   //! Write an 8-bit value at an absolute offset into the stack
   void write8(size_t offset, uint8_t value)
   {
      if (offset >= size()) throw "stack fault";
      raw[offset] = value;
   }

   //! Write a 16-bit value at an absolute offset into the stack
   void write16(size_t offset, uint16_t value)
   {
      if ((size() < 2) || (offset > (size() - 2))) throw "stack fault";
      raw[offset    ] = uint8_t(value >> 8);
      raw[offset + 1] = uint8_t(value);
   }

   //! Write a 24-bit value at an absolute offset into the stack
   void write24(size_t offset, uint32_t value)
   {
      if ((size() < 3) || (offset > (size() - 3))) throw "stack fault";
      raw[offset    ] = uint8_t(value >> 16);
      raw[offset + 1] = uint8_t(value >>  8);
      raw[offset + 2] = uint8_t(value);
   }

   //! Write a 32-bit value at an absolute offset into the stack
   void write32(size_t offset, uint32_t value)
   {
      if ((size() < 4) || (offset > (size() - 4))) throw "stack fault";
      raw[offset    ] = uint8_t(value >> 24);
      raw[offset + 1] = uint8_t(value >> 16);
      raw[offset + 2] = uint8_t(value >>  8);
      raw[offset + 3] = uint8_t(value);
   }

   //! Get 8-bit value at the top of the stack (without poping)
   uint8_t peek8() const { return read8(size() - 1); }

   //! Get 16-bit value at the top of the stack (without poping)
   uint16_t peek16() const { return read16(size() - 2); }

   //! Get 24-bit value at the top of the stack (without poping)
   uint32_t peek24() const { return read24(size() - 3); }

   //! Get 32-bit value at the top of the stack (without poping)
   uint32_t peek32() const { return read32(size() - 4); }

   //! Make stack empty
   void clear() { return raw.clear(); }

   //! Shrink the stack to a new size
   void shrink(size_t new_size)
   {
      if (new_size >= size()) throw "stack overflow";
      return raw.resize(new_size);
   }

   //! Push an 8-bit value onto the stack
   void push8(uint8_t value)
   {
      assert(size() <= max_size);
      if (size() == max_size) throw "stack overflow";
      return raw.push_back(value);
   }

   //! Push a 16-bit value onto the stack
   void push16(uint16_t value)
   {
      push8(value >> 8);
      push8(value & 0xFF);
   }

   //! Push a 24-bit value onto the stack
   void push24(uint32_t value)
   {
      push8(uint8_t(value >> 16));
      push16(value & 0xFFFF);
   }

   //! Push a 32-bit value onto the stack
   void push32(uint32_t value)
   {
      push16(value >> 16);
      push16(value & 0xFFFF);
   }

   //! Remove top 8-bit value from the stack
   uint8_t pop8()
   {
      uint8_t value = raw.back();
      raw.pop_back();
      return value;
   }

   //! Remove top 16-bit value from the stack
   uint16_t pop16()
   {
      uint16_t value = pop8();
      return (uint16_t(pop8()) << 8) | value;
   }

   //! Remove top 24-bit value from the stack
   uint32_t pop24()
   {
      uint32_t value = pop16();
      return (uint32_t(pop8()) << 16) | value;
   }

   //! Remove top 32-bit value from the stack
   uint32_t pop32()
   {
      uint32_t value = pop16();
      return (uint32_t(pop16()) << 16) | value;
   }

protected:
   size_t               max_size;
   std::vector<uint8_t> raw;
};

}

#endif
