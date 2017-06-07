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
#include "ZError.h"


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
private:

   // Terminal state
   mutable bool          do_quit{false};
   mutable ZError        err{NO_ERROR};

public:
   //! Return whether the machine should stop
   bool isQuitRequested() const { return do_quit; }

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

      if ((game_start >= game_end)   ||
          (game_end >= memory_limit) ||
          (global_base >= memory_limit))
      {
         error(ERR_BAD_CONFIG);
      }

      memory.setLimit(memory_limit);
   }

   //! Reset the dynamic state to the initial conditions
   //  This includes loading the body of the game file
   bool reset(const char* filename, uint32_t pc_, uint16_t header_checksum)
   {
      do_quit = false;

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
      bool ok = false;

      pushContext();

      FILE* fp = fopen("zif.save", "w");
      if (fp != nullptr)
      {
         memory.save(fp, game_start, memory_limit);
         fwrite(&stack, sizeof(stack), 1, fp);
         fclose(fp);

         ok = true;
      }

      popContext();

      return ok;
   }

   //! Restore the dynamic state from a save file
   bool restore()
   {
      bool ok = false;

      FILE* fp = fopen("zif.save", "r");
      if (fp != nullptr)
      {
         memory.load(fp, game_start, memory_limit);
         fread(&stack, sizeof(stack), 1, fp);
         fclose(fp);

         ok = true;
      }

      if (ok)
      {
         popContext();
      }

      return ok;
   }

   void quit() const
   {
      do_quit = true;
   }

   //! Report an error, terminates the machine
   bool error(ZError err_) const
   {
      // Only the first error is recorded
      if (err == NO_ERROR)
      {
         err = err_;
         quit();
      }

      return false;
   }

   //! Get the first error code reported
   ZError getError() const { return err; }

   //! Fetch an instruction byte
   uint8_t fetchByte()
   {
      return validatePC() ? memory.fetchByte(pc) : 0;
   }

   //! Fetch an instruction word
   uint16_t fetchWord()
   {
      return validatePC() ? memory.fetchWord(pc) : 0;
   }


   //! Push a word onto the stack
   void push(uint16_t value)
   {
      if (stack.full())
      {
         error(ERR_STACK_OVERFLOW);
         return;
      }

      stack.push_back(value);
   }

   //! Pop a word from the stack
   uint16_t pop()
   {
      if (stack.empty())
      {
         error(ERR_STACK_UNDERFLOW);
         return 0;
      }

      uint16_t value = stack.back();
      stack.pop_back();
      return value;
   }

   //! Peep word at the top of the stack
   uint16_t& peek()
   {
      if (stack.empty())
      {
         static uint16_t dummy;
         error(ERR_STACK_EMPTY);
         return dummy;
      }

      return stack.back();
   }

   uint16_t getNumFrameArgs() const
   {
      return validateFramePtr() ? stack[frame_ptr]
                                : 0;
   }


   //! Absolute jump to given target address
   void jump(uint32_t target_)
   {
      pc = target_;

      // Check PC here to catch current instruction address
      validatePC();
   }

   //! Jump relative to the current PC
   void branch(int16_t offset_)
   {
      pc += offset_;

      // Check PC here to catch current instruction address
      validatePC();
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

   //! Return from the given frame (usually the current frame)
   uint16_t returnFromFrame(uint32_t frame_ptr_)
   {
      if (!validateFramePtr()) return /* bad_call_type*/ 3;

      stack.resize(frame_ptr_);

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
   uint16_t varRead(uint8_t index, bool do_peek = false)
   {
      if (index == 0)
      {
         return do_peek ? peek() : pop();
      }
      else if (index < 16)
      {
         return validateFramePtr(index) ? stack[frame_ptr + index]
                                        : 0;
      }
      else
      {
         uint32_t addr = global_base + (index - 16) * 2;
         return validateAddr(addr) ? memory.readWord(addr)
                                   : 0;
      }
   }

   //! Write a variable
   void varWrite(uint8_t index, uint16_t value, bool do_peek = false)
   {
      if (index == 0)
      {
         if (do_peek)
            peek() = value;
         else
            push(value);
      }
      else if (index < 16)
      {
         if (!validateFramePtr(index)) return;
         stack[frame_ptr + index] = value;
      }
      else
      {
         uint32_t addr = global_base + (index - 16) * 2;
         if (!validateAddr(addr)) return;
         memory.writeWord(addr, value);
      }
   }

private:
   //! Range check PC
   bool validatePC() const
   {
      // TODO maybe (pc >= game_start) && (pc < game_end)
      // if self-modifying code is excluded
      return (pc >= game_start) && (pc < memory_limit) ? true
                                                       : error(ERR_BAD_PC);
   }

   //! Range check frame pointer
   bool validateFramePtr(uint16_t offset=0) const
   {
      return (frame_ptr > 0) && ((frame_ptr + offset) < stack.size()) ? true
                                                                      : error(ERR_BAD_FRAME_PTR);
   }

   //! Range check address
   bool validateAddr(uint32_t addr) const
   {
      return addr < memory_limit ? true
                                 : error(ERR_BAD_ADDRESS);
   }

   //! Save dynamic registers on the stack
   void pushContext()
   {
      push(checksum_ok);
      push32(rand_state);
      push32(pc);
      push(frame_ptr);
   }

   //! Restore dynamic registers from the stack
   void popContext()
   {
      frame_ptr   = pop();
      pc          = pop32();
      rand_state  = pop32();
      checksum_ok = pop();

      validateFramePtr();
      validatePC();
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
