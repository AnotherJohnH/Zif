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

#ifndef ZDECODER_H
#define ZDECODER_H


class ZDecoder
{
public:
   enum OperandFormat : uint8_t
   {
      NONE,
      ONE_WORD_CONST,
      ONE_BYTE_CONST,
      ONE_VARIABLE,
      TWO_BYTE_BYTE,
      TWO_BYTE_VAR,
      TWO_VAR_BYTE,
      TWO_BYTE_BYTE,
      VAR4,
      VAR8
   };

   ZDecoder()
   {
      for(unsigned op = 0; op < 256; op++)
      {
         if(op < 0x80)
         {
            opcode&(1 << 6) ? OP_VARIABLE : OP_SMALL_CONST;
            opcode&(1 << 5) ? OP_VARIABLE : OP_SMALL_CONST;
         }
         else if(op < 0xB0)
         {
            fetchOperand((opcode >> 4) & 3);
         }
         else if(op < 0xC0)
         {
            if(op == 0xBE)
            {
               op_format[op] = VAR4;
            }
            else
            {
               op_format[op] = NONE;
            }
         }
         else if(opcode < 0xE0)
         {
            op_format[i] = VAR4;
         }
         else
         {
            if((opcode == 0xEC) || (opcode == 0xFA))
               op_format = VAR8;
            else
               op_format = VAR4;
         }
      }
   }

   OperandFormat getFormat(uint8_t op) const {}

private:
   OperandFormat op_format[256];
};

#endif
