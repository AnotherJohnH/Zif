//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

#include "AddHexString.h"

namespace IF {

//! Z Disassembler
class Disassembler
{
public:
   Disassembler() = default;

   //! Disassemble a single op
   unsigned disassemble(std::string& text, uint32_t inst_addr, const uint8_t* raw) const
   {
      text = "";

      if (in_trace)
      {
         addHexString(text, trace_count++, 6, ' ');
         text += "  ";
      }

      addHexString(text, inst_addr, 6);
      text += "  ";

      unsigned n = decodeOp(work, raw, !in_trace);

      if (in_trace)
      {
         for(unsigned i=0; i<10; i++)
         {
            if (i < n)
            {
               addHexString(text, raw[i], 2);
               text += " ";
            }
            else
            {
               text += "   ";
            }
         }
      }
      else
      {
         text += "  ";
      }

      text += work;

      return n;
   }

   //! Trace an op at the given address
   unsigned trace(std::string& text, uint32_t inst_addr, const uint8_t* raw)
   {
      in_trace = true;

      unsigned n = disassemble(text, inst_addr, raw);
      text += "\n";

      in_trace = false;
      return n;
   }

protected:
   // Local strings to avoid repeated dynamic allocation
   mutable std::string work{};

   bool             in_trace{false};
   mutable unsigned trace_count{0};

   virtual unsigned decodeOp(std::string& text, const uint8_t* raw, bool pack) const = 0;
};

} // namespace IF

