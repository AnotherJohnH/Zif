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

#ifndef IF_STATE_H
#define IF_STATE_H

#include <cstdint>
#include <string>

#include "common/Memory.h"
#include "common/Random.h"
#include "common/Stack.h"
#include "common/Story.h"

namespace IF {

//! Machine state base class for an interactive fiction VM
class State
{
public:
   State(const IF::Story&   story_,
         uint32_t           initial_rand_seed_,
         Stack::Offset      stack_size_)
      : story(story_)
      , initial_rand_seed(initial_rand_seed_)
      , stack(stack_size_)
   {
      story_.prepareMemory(memory);
   }

   //! Return whether the machine should stop
   bool isQuitRequested() const { return do_quit; }

   //! Current value of the program counter
   Memory::Address getPC() const { return pc; }

   //! Current value of the frame pointer
   Stack::Offset getFramePtr() const { return frame_ptr; }

   //! Reset the dynamic state to the initial conditions
   void reset()
   {
      do_quit   = false;
      pc        = story.getEntryPoint();
      frame_ptr = 0;

      story.resetMemory(memory);

      stack.clear();

      if (initial_rand_seed != 0)
      {  
         random.predictableSeed(initial_rand_seed);
      }
   }

   //! Fetch an instruction byte
   uint8_t fetch8()
   {
      return memory.fetch8(pc++);
   }

   //! Fetch a 16-bit instruction word
   uint16_t fetch16()
   {
      uint16_t word = memory.fetch16(pc);
      pc += 2;
      return word;
   }

   //! Fetch a 24-bit instruction word
   uint32_t fetch24()
   {
      uint32_t word = memory.fetch24(pc);
      pc += 3;
      return word;
   }

   //! Fetch a 32-bit instruction word
   uint32_t fetch32()
   {
      uint32_t word = memory.fetch32(pc);
      pc += 4;
      return word;
   }

   //! Absolute jump to given target address
   void jump(Memory::Address target)
   {
      pc = target;
   }

   //! Jump relative to the current PC
   void branch(signed offset)
   {
      pc += offset;
   }

   //! Signal exit
   void quit()
   {
      do_quit = true;
   }

protected:
   // Configuration
   const IF::Story& story;
   const uint32_t   initial_rand_seed{0};

   // Dynamic state
   bool            do_quit{false};
   Memory::Address pc{0};
public:
   Stack::Offset   frame_ptr{0};
   IF::Memory      memory;
   IF::Stack       stack;
   IF::Random      random;
};

} // namespace IF

#endif
