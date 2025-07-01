//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "Level9/State.h"
#include "Level9/Story.h"

namespace Level9 {

//! Level9 machine implementation
class Machine
{
public:
   Machine(Console& console_, Options& options_, Level9::Story& story_)
      : console(console_)
      , options(options_)
      , story(story_)
   {
   }

   bool play(bool restore)
   {
      (void) options;
      (void) story;
      console.error("Level9 games are not currently supported");
      console.waitForKey();
      return false;
   }

private:
   Console&       console;
   const Options& options;
   const Story&   story;
   State          state;
};

} // namespace Level9

