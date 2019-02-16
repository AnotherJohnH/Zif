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

#ifndef GLULX_STATE_H
#define GLULX_STATE_H

#include <cstring>
#include <vector>

#include "share/Memory.h"
#include "share/Random.h"

#include "Glulx/Story.h"

namespace Glulx {

//! Glulx machine implementation
class State
{
public:
   State(const Story&       story_,
         const std::string& save_dir_,
         unsigned           num_undo,
         uint32_t           initial_rand_seed_)
      : story(story_)
      , save_dir(save_dir_)
      , initial_rand_seed(initial_rand_seed_)
   {
      const Header* header = story.getHeader();

      memory.resize(header->end_mem);
      stack.reserve(header->stack_size);
   }

   //! Return whether the machine should stop
   bool isQuitRequested() const { return quit; }

   //! Reset the dynamic state to the initial conditions.
   void reset()
   {
      const Header* header = story.getHeader();

      quit = false;

      if (initial_rand_seed != 0)
      {
         random.seed(initial_rand_seed);
      }

      pc        = header->start_func;
      frame_ptr = 0;

      memcpy(memory.data(), story.data(), story.size());
      memset(memory.data() + header->ext_start, 0, header->end_mem - header->ext_start);

      stack.clear();
   }

private:
   // Static configuration
   const Story&         story;
   std::string          save_dir;
   uint32_t             initial_rand_seed{0};

   // Dynamic state
   bool                 quit{false};
   uint32_t             pc{0};
   uint32_t             frame_ptr{0};
   IF::Memory           memory;
   std::vector<uint8_t> stack;
   IF::Random           random;
};

} // namespace Glulx

#endif
