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

#ifndef ZSTACK_H
#define ZSTACK_H

#include <cassert>
#include <stdint.h>

#include "STB/Stack.h"

//! Stack for ZMachine
template <unsigned SIZE>
class ZStack
{
public:
   void reset()
   {
      impl.clear();
      fp = 0;
   }


   void push(uint16_t value)
   {
      impl.push_back(value);
   }

   uint16_t& peek()
   {
      return impl.back();
   }

   uint16_t pop()
   {
      uint16_t value = impl.back();
      impl.pop_back();
      return value;
   }


   //! Start a new call-frame
   void pushFrame(uint16_t call_type, uint32_t pc, uint16_t num_arg)
   {
      push(call_type);
      push32(pc);
      push(fp);
      fp = impl.size();
      push(num_arg);
   }

   //! End the current call-frame
   void popFrame(uint16_t& call_type, uint32_t& pc)
   {
      assert(fp != 0);
      impl.resize(fp);
      fp = pop();
      pc = pop32();
      call_type = pop();
   }


   //! Returns the current frame pointer
   uint16_t getFramePtr() const { return fp; }

   uint16_t getNumFrameArgs() const
   {
      assert(fp != 0);
      return impl[fp];
   }

   uint16_t& peekFrame(unsigned index)
   {
      assert(index <= 14);
      return impl[fp + 1 + index];
   }

   void push32(uint32_t value)
   {
      push(uint16_t(value));
      push(uint16_t(value >> 16));
   }

   uint32_t pop32()
   {
      uint32_t value = pop()<<16;
      return value | pop();
   }

private:
   STB::Stack<uint16_t,SIZE,uint16_t>   impl;
   uint16_t                             fp{};
};

#endif
