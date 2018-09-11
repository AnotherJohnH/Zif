//------------------------------------------------------------------------------
// Copyright (c) 2016-2017 John D. Haughton
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

#ifndef ZDISASSEMBLER_H
#define ZDISASSEMBLER_H

#include <cstdint>
#include <cstdio>

enum ZOperandType : uint8_t
{
   OP_LARGE_CONST = 0,
   OP_SMALL_CONST = 1,
   OP_VARIABLE    = 2,
   OP_NONE        = 3
};


//!
// XXX this is all a bit unsafe and nasty - probably should
// bite the bullet and alow dynamic allocation
class ZDisassembler
{
private:
   //! Disassemble a variable operand
   void disVar(uint8_t index, char* text)
   {
      if (index == 0)
      {
         strcpy(text, " [SP]");
      }
      else if (index < 16)
      {
         sprintf(text, " [FP+%u]", index);
      }
      else
      {
         sprintf(text, " G%u", index - 16);
      }
   }

   //! Disassemble an operand
   unsigned disOp(unsigned op, const uint8_t* code, char* text)
   {
      switch(op)
      {
      case OP_LARGE_CONST: sprintf(text, " #0x%02X%02X", code[0], code[1]); return 2;
      case OP_SMALL_CONST: sprintf(text, " #0x%02X",     code[0]);          return 1;
      case OP_VARIABLE:    disVar(code[0], text);                           return 1;
      case OP_NONE: break;
      }

      return 0;
   }

   //! Disassemble a variable number of operands
   unsigned disOperands(unsigned n, const uint8_t* code, char* text)
   {
      unsigned bytes    = 1;
      uint16_t op_types = (*code++) << 8;

      if(n == 8)
      {
         op_types |= *code++;
         bytes += 1;
      }

      // Unpack the type of the operands
      for(unsigned i = 0; i < n; ++i)
      {
         ZOperandType type = ZOperandType(op_types >> 14);
         if(type == OP_NONE) break;

         unsigned n = disOp(type, code, text);
         bytes += n;
         code  += n;
         text  += strlen(text);

         op_types <<= 2;
      }

      return bytes;
   }

   unsigned disOp0(const uint8_t* code, char* text)
   {
      uint8_t opcode = code[0];

      sprintf(text, "0OP-%1X", opcode & 0xF);

      return 1;
   }

   unsigned disOp1(const uint8_t* code, char* text)
   {
      uint8_t opcode = code[0];

      sprintf(text, "1OP-%1X  ", opcode & 0xF);
      text += strlen(text);

      return 1 + disOp((opcode >> 4) & 3, code + 1, text);
   }

   unsigned disOp2(const uint8_t* code, char* text)
   {
      uint8_t opcode = code[0];

      sprintf(text, "2OP-%02X ", opcode & 0x1F);
      text += strlen(text);

      disOp((opcode >> 6) & 1 ? OP_VARIABLE : OP_SMALL_CONST, code + 1, text);
      text += strlen(text);

      disOp((opcode >> 6) & 1 ? OP_VARIABLE : OP_SMALL_CONST, code + 2, text);

      return 3;
   }

   unsigned disOp2_var(const uint8_t* code, char* text)
   {
      uint8_t opcode = code[0];

      sprintf(text, "2OP-%02X ", opcode & 0x1F);
      text += strlen(text);

      return 1 + disOperands(4, code + 1, text);
   }

   unsigned disOpV(const uint8_t* code, char* text)
   {
      uint8_t opcode = code[0];

      sprintf(text, "VAR-%02X ", opcode & 0x1F);
      text += strlen(text);

      if((opcode == 0xEC) || (opcode == 0xFA))
      {
         return 1 + disOperands(8, code + 1, text);
      }

      return 1 + disOperands(4, code + 1, text);
   }

   unsigned disOpE(const uint8_t* code, char* text)
   {
      uint8_t opcode = code[1];

      sprintf(text, "EXT-%02X ", opcode & 0x1F);
      text += strlen(text);

      return 2 + disOperands(4, code + 2, text);
   }

public:
   unsigned disassemble(uint16_t inst_addr, const uint8_t* code, char* text)
   {
      unsigned n = 1;

      uint8_t opcode = *code;
      
      sprintf(text, "%06X  %02X ", inst_addr, opcode);
      text += strlen(text);
      
      if(opcode < 0x80)
      {
         n = disOp2(code, text);
      }
      else if(opcode < 0xB0)
      {
         n = disOp1(code, text);
      }
      else if(opcode < 0xC0)
      {
         if((opcode & 0xF) == 0xE)
         {
            n = disOpE(code, text);
         }
         else
         {
            n = disOp0(code, text);
         }
      }
      else if(opcode < 0xE0)
      {
         n = disOp2_var(code, text);
      }
      else
      {
         n = disOpV(code, text);
      }

      return n;
   }
};

#endif
