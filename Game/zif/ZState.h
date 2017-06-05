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

#ifndef ZSTATE_H
#define ZSTATE_H

#include <cstdio>

#include "ZMemory.h"
#include "ZStack.h"

//! The game state
class ZState
{
private:
   static const unsigned STACK_SIZE = 1024;

   // Static configuration
   uint32_t              memory_limit{0};
   uint32_t              game_start{0};
   uint32_t              game_end{0};

   // Dynamic state
   bool                  checksum_ok{false};
   uint32_t              rand_state{1};
   uint32_t              pc{0};
public:
   ZStack<STACK_SIZE>    stack;
   ZMemory               memory;

   //! Return whether the loaded checksum was valid
   bool isChecksumOk() const { return checksum_ok; }

   //! Current value of program counter
   uint32_t getPC() const { return pc; }

   //! Initialise with the game configuration
   void init(uint32_t game_start_,
             uint32_t game_end_,
             uint32_t memory_limit_)
   {
      game_start   = game_start_;
      game_end     = game_end_;
      memory_limit = memory_limit_;

      memory.setLimit(memory_limit_);
   }

   //! Reset the dynamic state to the initial conditions
   //  This includes loading the body of the game file
   bool reset(const char* filename, uint32_t pc_, uint16_t header_checksum)
   {
      rand_state = 1;

      jump(pc_);

      stack.reset();

      memory.clear(game_end, memory_limit);

      FILE* fp = fopen(filename, "r");
      if (fp == NULL)
      {
         return false;
      }

      // skip the header
      fseek(fp, game_start, SEEK_SET);

      uint16_t calculated_checksum;

      if (!memory.load(fp, game_start, game_end, &calculated_checksum))
      {
         return false;
      }

      fclose(fp);

      checksum_ok = calculated_checksum == header_checksum;

      return true;
   }

   //! Save the dynamic state to a file
   bool save()
   {
      FILE* fp = fopen("zif.save", "w");
      if (fp != nullptr) return false;

      pushContext();

      memory.save(fp, game_start, memory_limit);
      stack.save(fp);
      fclose(fp);

      popContext();

      return true;
   }

   //! Restore the dynamic state from a save file
   bool restore()
   {
      FILE* fp = fopen("zif.save", "r");
      if (fp == nullptr) return false;

      memory.load(fp, game_start, memory_limit);
      stack.load(fp);
      fclose(fp);

      popContext();

      return true;
   }

   //! Fetch an instruction byte
   uint8_t fetchByte()
   {
      return memory.fetchByte(pc);
   }

   //! Fetch an instruction word
   uint16_t fetchWord()
   {
      return memory.fetchWord(pc);
   }

   //! Absolute jump to given target address
   void jump(uint32_t target_)
   {
      pc = target_;
   }

   //! Jump relative to the current PC
   void branch(int16_t offset_)
   {
      pc += offset_;
   }

   uint16_t random(int16_t arg)
   {
      if (arg <= 0)
      {
         arg = -arg;

         if (arg == 0)
         {
            // TODO
         }
         else if (arg < 1000)
         {
            // TODO
         }
         else
         {
            rand_state = arg;
         }

         return 0;
      }
      else
      {
         rand_state = 0x015A4E35 * rand_state + 1;
         uint16_t value = (rand_state >> 16) & 0x7FFF;
         return (value % arg) + 1;
      }
   }

private:
   //! Save all registers on the stack
   void pushContext()
   {
      stack.push(checksum_ok);
      stack.push32(rand_state);
      stack.push32(pc);
   }

   //! Restore all registers from the stack
   void popContext()
   {
      pc          = stack.pop32();
      rand_state  = stack.pop32();
      checksum_ok = stack.pop();
   }
};

#endif
