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

#include "STB/Stack.h"
#include "PLT/File.h"

#include "share/Error.h"
#include "share/State.h"
#include "share/Random.h"
#include "share/Stack.h"

#include "ZHeader.h"
#include "ZMemory.h"
#include "ZQuetzal.h"
#include "Z/Story.h"

//! Z machine state
class ZState : public IF::State
{
private:
   static const uint32_t GAME_START = sizeof(ZHeader);

   // Static configuration
   const Z::Story&  story;
   uint32_t         game_end{0};
   uint32_t         global_base{0};

   // Saved state
   ZQuetzal              save_file;
   std::vector<ZQuetzal> undo;
   unsigned              undo_oldest{0};
   unsigned              undo_next{0};

public:
   ZMemory    memory;

private:
   // Terminal state
   mutable Error exit_code{Error::NONE};

public:
   ZState(const Z::Story&    story_,
          const std::string& save_dir_,
          unsigned           num_undo,
          uint32_t           initial_rand_seed_)
      : IF::State(save_dir_, initial_rand_seed_, 2048)
      , story(story_)
   {
      undo.resize(num_undo);

      const ZHeader* header = story.getHeader();

      game_end     = header->getStorySize();
      global_base  = header->glob;

      if (!memory.init(story))
      {
         error(Error::BAD_CONFIG);
      }
   }

   //! Reset the dynamic state to the initial conditions.
   void reset()
   {
      IF::State::reset(story.getHeader()->getEntryPoint());

      memory.reset(story);
   }

   //! Save the dynamic state to a file
   bool save(const std::string& name = "")
   {
      pushContext();
      save_file.encode(story, pc, memory, stack, random);
      popContext();

      // Make sure the save directory exists
      (void) PLT::File::createDir(save_dir.c_str());

      std::string path = save_dir;
      path += '/';
      path += name == "" ? story.getFilename() : name;
      path += ".qzl";

      return save_file.write(path);
   }

   //! Restore the dynamic state from a save file
   bool restore(const std::string& name = "")
   {
      std::string path = save_dir;
      path += '/';
      path += name == "" ? story.getFilename() : name;
      path += ".qzl";

      if (save_file.read(path) &&
          save_file.decode(story, pc, memory, stack, random))
      {
         validatePC();
         popContext();
         return true;
      }

      return false;
   }

   //! Save the dynamic state into the undo buffer
   bool saveUndo()
   {
      if (undo.size() == 0) return false;

      pushContext();
      undo[undo_next].encode(story, pc, memory, stack, random);
      popContext();

      undo_next = (undo_next + 1) % undo.size();
      if (undo_next == undo_oldest)
      {
         undo_oldest = (undo_oldest + 1) % undo.size();
      }

      return true;
   }

   //! Restore the dynamic state from the undo buffer
   bool restoreUndo()
   {
      if (undo_next == undo_oldest) return false;

      undo_next = undo_next == 0 ? undo.size() - 1
                                 : undo_next - 1;

      undo[undo_next].decode(story, pc, memory, stack, random);
      popContext();

      return true;
   }

   //! Report an error, terminates the machine
   bool error(Error err_) const
   {
      // Only the first error is recorded
      if(!isError(exit_code))
      {
         exit_code = err_;
         ((State*)this)->quit();
      }

      return false;
   }

   //! Get the first error code reported
   Error getExitCode() const { return exit_code; }

   //! Fetch an instruction byte
   uint8_t fetchByte()
   {
      assert(validatePC());
      return memory.fetch8(pc++);
   }

   //! Fetch an instruction word
   uint16_t fetchWord()
   {
      assert(validatePC());
      uint16_t word = memory.fetch16(pc);
      pc += 2;
      return word;
   }


   //! Push a word onto the stack
   void push(uint16_t value) { stack.push16(value); }

   //! Pop a word from the stack
   uint16_t pop() { return stack.pop16(); }


   uint16_t getNumFrameArgs() const
   {
      return validateFramePtr() ? stack.read16(frame_ptr)
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
      if(!validateFramePtr()) return /* bad_call_type*/ 3;

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
         random.seed(-arg);
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
         return validateFramePtr(index) ? stack.read16(frame_ptr + 2*index)
                                        : 0;
      }
      else
      {
         uint32_t addr = global_base + (index - 16) * 2;
         return validateAddr(addr) ? memory.read16(addr)
                                   : 0;
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
         if(!validateFramePtr(index)) return;
         stack.write16(frame_ptr + 2*index, value);
      }
      else
      {
         uint32_t addr = global_base + (index - 16) * 2;
         if(!validateAddr(addr)) return;
         memory.write16(addr, value);
      }
   }

private:
   //! Range check PC
   bool validatePC() const
   {
      return (pc >= GAME_START) && (pc < game_end) ? true : error(Error::BAD_PC);
   }

   //! Range check frame pointer
   bool validateFramePtr(uint16_t offset = 0) const
   {
      return (frame_ptr > 0) && ((frame_ptr + offset) < stack.size()) ? true
                                                                      : error(Error::BAD_FRAME_PTR);
   }

   //! Range check address
   bool validateAddr(uint32_t addr) const
   {
      return addr < game_end ? true : error(Error::BAD_ADDRESS);
   }

   //! Save dynamic registers on the stack
   void pushContext()
   {
      stack.push16(frame_ptr);
   }

   //! Restore dynamic registers from the stack
   void popContext()
   {
      frame_ptr = stack.pop16();
      validateFramePtr();
   }
};

#endif
