//-------------------------------------------------------------------------------
// Copyright (c) 2018 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

