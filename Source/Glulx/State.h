//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "common/SavableState.h"

#include "Glulx/Story.h"

namespace Glulx {

//! Glulx machine implementation
class State : public IF::SavableState
{
public:
   State(const Story&       story_,
         const std::string& save_dir_,
         unsigned           num_undo_,
         uint32_t           initial_rand_seed_)
      : IF::SavableState(story_,
                         save_dir_,
                         num_undo_,
                         initial_rand_seed_,
                         story_.getHeader()->stack_size)
   {
   }

private:
   virtual void pushContext() override
   {}

   virtual void popContext() override
   {}
};

} // namespace Glulx

