//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "STB/Stack.h"
#include "PLT/File.h"

#include "common/SavableState.h"

#include "Z/Header.h"
#include "Z/Story.h"

namespace Z {

//! Z machine state
class State : public IF::SavableState
{
public:
   State(const Z::Story&    story_,
         const std::string& save_dir_,
         unsigned           num_undo_,
         uint32_t           initial_rand_seed_)
      : IF::SavableState(story_, save_dir_, num_undo_, initial_rand_seed_, 2048)
   {
      const Header* header = story_.getHeader();
      global_base = header->glob;
   }


   //! Push a word onto the stack
   void push(uint16_t value) { stack.push16(value); }

   //! Pop a word from the stack
   uint16_t pop() { return stack.pop16(); }


   uint16_t getNumFrameArgs() const
   {
      return stack.read16(frame_ptr);
   }

   //! Call a routine
   void call(uint8_t call_type, uint32_t target)
   {
      stack.push8(call_type);
      stack.push24(pc);
      stack.push16(frame_ptr);

      frame_ptr = stack.size();

      jump(target);
   }

   //! Return from the given frame (usually the current frame)
   uint8_t returnFromFrame(uint32_t frame_ptr_)
   {
      stack.shrink(frame_ptr_);

      frame_ptr = stack.pop16();
      jump(stack.pop24());
      return stack.pop8();
   }

   //! Implement random op
   uint16_t randomOp(int16_t arg)
   {
      if(arg == 0)
      {
         random.unpredictableSeed();
         return 0;
      }
      else if(arg < 0)
      {
         if (-arg < 1000)
         {
            random.sequentialSeed(-arg);
         }
         else
         {
            random.predictableSeed(-arg);
         }
         return 0;
      }
      else
      {
         uint16_t value = (random.get() >> 16) & 0x7FFF;
         return (value % arg) + 1;
      }
   }

   //! Read a variable
   uint16_t varRead(uint8_t index, bool do_peek = false)
   {
      if(index == 0)
      {
         return do_peek ? stack.peek16() : stack.pop16();
      }
      else if(index < 16)
      {
         return stack.read16(frame_ptr + 2*index);
      }
      else
      {
         return memory.read16(global_base + (index - 16) * 2);
      }
   }

   //! Write a variable
   void varWrite(uint8_t index, uint16_t value, bool do_peek = false)
   {
      if(index == 0)
      {
         if(do_peek)
            stack.write16(stack.size() - 2, value);
         else
            push(value);
      }
      else if(index < 16)
      {
         stack.write16(frame_ptr + 2*index, value);
      }
      else
      {
         memory.write16(global_base + (index - 16) * 2, value);
      }
   }

private:
   //! Save dynamic registers on the stack
   virtual void pushContext() override
   {
      stack.push16(frame_ptr);
   }

   //! Restore dynamic registers from the stack
   virtual void popContext() override
   {
      frame_ptr = stack.pop16();
   }

private:
   // Static configuration
   uint32_t global_base{0};
};

} // namespace Z

