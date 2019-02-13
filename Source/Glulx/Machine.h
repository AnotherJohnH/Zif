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

#ifndef GLULX_MACHINE_H
#define GLULX_MACHINE_H

#include "Glulx/State.h"
#include "Glulx/Story.h"

namespace Glulx {

//! Glulx machine implementation
class Machine
{
public:
   Machine(Console& console_, const Options& options_, const Glulx::Story& story_)
      : console(console_)
      , options(options_)
      , story(story_)
   {
   }

   bool play()
   {
      (void) options;
      (void) story;
      console.error("Glulx games are not currently supported");
      console.waitForKey();
      return false;
   }

private:
   Console&       console;
   const Options& options;
   const Story&   story;
   State          state;
};

} // namespace Glulx

#endif
