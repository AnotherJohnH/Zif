//------------------------------------------------------------------------------
// Copyright (c) 2016-2018 John D. Haughton
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

#ifndef IF_DISASSEMBLER_H
#define IF_DISASSEMBLER_H

#include <cstdint>
#include <string>

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
         fmtHex(text, trace_count++, 6, ' ');
         text += "  ";
      }

      fmtHex(text, inst_addr, 6);
      text += "  ";

      unsigned n = decodeOp(work, raw);

      for(unsigned i=0; i<10; i++)
      {
         if (i < n)
         {
            fmtHex(text, raw[i], 2);
            text += " ";
         }
         else
         {
            text += "   ";
         }
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

   virtual unsigned decodeOp(std::string& text, const uint8_t* raw) const = 0;

   // Not entirely sure why I needed to write this!
   template <typename TYPE>
   static void fmtHex(std::string& text, TYPE value, unsigned digits = 0, char pad = '0')
   {
      for(signed n = sizeof(TYPE) * 2 - 1; n >= 0; n--)
      {
         unsigned digit = (value >> (n * 4)) & 0xF;

         if ((n != 0) && (digit == 0))
         {
            if ((value >> (n * 4)) != 0)
            {
               text += '0';
            }
            else if (n <= signed(digits))
            {
               text += pad;
            }
         }
         else
         {
            text += digit > 9 ? 'A' + digit - 10
                              : '0' + digit;
         }
      }
   }
};

} // namespace IF

#endif
