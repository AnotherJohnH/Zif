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
#include "share/State.h"

#include "Glulx/Story.h"

namespace Glulx {

//! Glulx machine implementation
class State : public IF::State
{
public:
   State(const Story&       story_,
         const std::string& save_dir_,
         unsigned           num_undo,
         uint32_t           initial_rand_seed_)
      : IF::State(save_dir_, initial_rand_seed_, story.getHeader()->stack_size)
      , story(story_)
   {
      story.prepareMemory(memory);
   }

   //! Reset the dynamic state to the initial conditions.
   void reset()
   {
      const Header* header = story.getHeader();
      IF::State::reset(header->start_func);
   }

private:
   // Static configuration
   const Story&  story;

   // Dynamic state
   IF::Memory    memory;
};

} // namespace Glulx

#endif
