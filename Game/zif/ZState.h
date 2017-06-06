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

#include "STB/Stack.h"

#include "ZMemory.h"

//! The game state
class ZState
{
private:
   static const unsigned STACK_SIZE = 1024;

   using Stack = STB::Stack<uint16_t,STACK_SIZE,uint16_t>;

   // Static configuration
   uint16_t              initial_rand_seed{0};
   uint32_t              game_start{0};
   uint32_t              game_end{0};
   uint32_t              global_base{0};
   uint32_t              memory_limit{0};

   // Dynamic state
   bool                  checksum_ok{false};
   uint32_t              rand_state{1};
   uint32_t              pc{0};
   uint16_t              frame_ptr{0};
   Stack                 stack;
public:
   ZMemory               memory;

   //! Return whether the loaded checksum was valid
   bool isChecksumOk() const { return checksum_ok; }

   //! Current value of the program counter
   uint32_t getPC() const { return pc; }

   //! Current value of the frame pointer 
   uint16_t getFramePtr() const { return frame_ptr; }

   //! Initialise with the game configuration
   void init(uint16_t initial_rand_seed_,
             uint32_t game_start_,
             uint32_t game_end_,
             uint32_t global_base_,
             uint32_t memory_limit_)
   {
      initial_rand_seed = initial_rand_seed_;

      game_start   = game_start_;
      game_end     = game_end_;
      global_base  = global_base_;
      memory_limit = memory_limit_;

      memory.setLimit(memory_limit_);
   }

   //! Reset the dynamic state to the initial conditions
   //  This includes loading the body of the game file
   bool reset(const char* filename, uint32_t pc_, uint16_t header_checksum)
   {
      random(-(initial_rand_seed & 0x7FFF));

      jump(pc_);

      frame_ptr = 0;

      stack.clear();

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
      if (fp == nullptr) return false;

      pushContext();

      memory.save(fp, game_start, memory_limit);
      fwrite(&stack, sizeof(stack), 1, fp);
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
      fread(&stack, sizeof(stack), 1, fp);
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


   //! Push a word onto the stack
   void push(uint16_t value)
   {
      stack.push_back(value);
   }

   //! Pop a word from the stack
   uint16_t pop()
   {
      uint16_t value = stack.back();
      stack.pop_back();
      return value;
   }

   uint16_t getNumFrameArgs() const
   {
      assert(frame_ptr != 0);
      return stack[frame_ptr];
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

   //! Call a routine
   void call(uint16_t call_type, uint32_t target)
   {
      push(call_type);
      push32(pc);
      push(frame_ptr);

      frame_ptr = stack.size();

      jump(target);
   }

   //! Return from a routine
   uint16_t callret()
   {
      stack.resize(frame_ptr);

      frame_ptr = pop();
      jump(pop32());
      return pop();
   }

   //! Get a random value
   uint16_t random(int16_t arg)
   {
      if (arg <= 0)
      {
         rand_state = arg == 0 ? randomSeed()
                               : -arg;
         return 0;
      }
      else
      {
         rand_state = 0x015A4E35 * rand_state + 1;
         uint16_t value = (rand_state >> 16) & 0x7FFF;
         return (value % arg) + 1;
      }
   }

   //! Read a variable
   uint16_t varRead(uint8_t index, bool peek = false)
   {
      if (index == 0)
      {
         return peek ? stack.back() : pop();
      }
      else if (index < 16)
      {
         return stack[frame_ptr + index];
      }
      else
      {
         uint32_t addr = global_base + (index - 16) * 2;
         return memory.readWord(addr);
      }
   }

   //! Write a variable
   void varWrite(uint8_t index, uint16_t value, bool peek = false)
   {
      if (index == 0)
      {
         if (peek)
            stack.back() = value;
         else
            push(value);
      }
      else if (index < 16)
      {
         stack[frame_ptr + index] = value;
      }
      else
      {
         uint32_t addr = global_base + (index - 16) * 2;
         memory.writeWord(addr, value);
      }
   }

private:
   //! Save all registers on the stack
   void pushContext()
   {
      push(checksum_ok);
      push32(rand_state);
      push32(pc);
      push(frame_ptr);
   }

   //! Restore all registers from the stack
   void popContext()
   {
      frame_ptr   = pop();
      pc          = pop32();
      rand_state  = pop32();
      checksum_ok = pop();
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

   uint32_t randomSeed()
   {
      // TODO re-seed with an unpredictable value
      return 1;
   }
};

#endif
