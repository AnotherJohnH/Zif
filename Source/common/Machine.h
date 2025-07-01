//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "common/Memory.h"
#include "common/Options.h"

namespace IF {

//!
class Machine
{
public:
   Machine(Console& console_, const Options& options_)
      : console(console_)
      , options(options_)
   {
   }

protected:
   Console&             console;
   const Options&       options;
   IF::Memory::Address  inst_addr{0};
   std::string          dis_text;
};

} // namespace IF

