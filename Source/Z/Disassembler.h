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

#ifndef Z_DISASSEMBLER_H
#define Z_DISASSEMBLER_H

#include <cstdint>
#include <cstdio>
#include <string>

namespace Z {

enum OperandType : uint8_t
{
   OP_LARGE_CONST = 0,
   OP_SMALL_CONST = 1,
   OP_VARIABLE    = 2,
   OP_NONE        = 3
};


//! Z Disassembler
class Disassembler
{
public:
   Disassembler(unsigned v = 5)
   {
      // Zero operand instructions
      declOp(0x0, "rtrue", '0');
      declOp(0x1, "rfalse", '0');
      declOp(0x2, "print", '0');
      declOp(0x3, "print_ret", '0');
      declOp(0x4, "nop", '0');
      if (v <= 4) declOp(0x5, "save",    '0');
      if (v <= 4) declOp(0x6, "restore", '0');
      declOp(0x7, "restart", '0');
      declOp(0x8, "ret_popped", '0');
      if (v <= 4) declOp(0x9, "pop", '0'); else declOp(0x9, "catch", '0');
      declOp(0xA, "quit", '0');
      declOp(0xB, "new_line", '0');
      if (v == 3) declOp(0xC, "show_status", '0'); else if (v > 3) declOp(0xC, "nop", '0');
      if (v >= 3) declOp(0xD, "verify", '0');
      if (v >= 5) declOp(0xF, "piracy", '0');
   }

   //! Disassemble a single instruction
   unsigned disassemble(std::string& text, uint16_t inst_addr, const uint8_t* code)
   {
      unsigned n = 0;

      uint8_t opcode = *code;
      if(opcode < 0x80)
      {
         n = disOp2(inst, code);
      }
      else if(opcode < 0xB0)
      {
         n = disOp1(inst, code);
      }
      else if(opcode < 0xC0)
      {
         if((opcode & 0xF) == 0xE)
         {
            n = disOpE(inst, code);
         }
         else
         {
            n = disOp0(inst, code);
         }
      }
      else if(opcode < 0xE0)
      {
         n = disOp2_var(inst, code);
      }
      else
      {
         n = disOpV(inst, code);
      }

      text = "";
      fmtHex(text, inst_addr, 6);
      text += "  ";

      for(unsigned i=0; i<10; i++)
      {
         if (i < n)
         {
            fmtHex(text, code[i], 2);
            text += " ";
         }
         else
         {
            text += "   ";
         }
      }

      text += inst;

      return n;
   }

private:
   // Local strings to avoid repeated dynamic allocation
   std::string inst{};

   void declOp(uint8_t code, const char* mnemonic, char type)
   {
   }

   //! Disassemble a variable operand
   void fmtVar(std::string& text, uint8_t index)
   {
      if (index == 0)
      {
         text += " [SP]";
      }
      else if (index < 16)
      {
         text += " [FP+";
         text += std::to_string(index);
         text += "]";
      }
      else
      {
         text += " G";
         text += std::to_string(index - 16);
      }
   }

   //! Disassemble an operand
   unsigned fmtOp(std::string& text, unsigned op_type, const uint8_t* code)
   {
      switch(op_type)
      {
      case OP_LARGE_CONST: text += " #0x"; fmtHex(text, code[0]); fmtHex(text, code[1]); return 2;
      case OP_SMALL_CONST: text += " #0x"; fmtHex(text, code[0]); return 1;
      case OP_VARIABLE:    fmtVar(text, code[0]); return 1;
      case OP_NONE:        return 0;
      }

      return 0;
   }

   //! Disassemble a variable number of operands
   unsigned fmtOperands(std::string& text, unsigned n, const uint8_t* code)
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
         OperandType type = OperandType(op_types >> 14);
         if(type == OP_NONE) break;

         unsigned n = fmtOp(text, type, code);
         bytes += n;
         code  += n;

         op_types <<= 2;
      }

      return bytes;
   }

   unsigned disOp0(std::string& text, const uint8_t* code)
   {
      text = "0OP-";
      fmtHex(text, code[0] & 0xF, 1);
      return 1;
   }

   unsigned disOp1(std::string& text, const uint8_t* code)
   {
      text = "1OP-";
      fmtHex(text, code[0] & 0xF, 1);
      text += "  ";
      return 1 + fmtOp(text, (code[0] >> 4) & 3, code + 1);
   }

   unsigned disOp2(std::string& text, const uint8_t* code)
   {
      text = "2OP-";
      fmtHex(text, code[0] & 0x1F, 2);
      text += " ";
      fmtOp(text, (code[0] >> 6) & 1 ? OP_VARIABLE : OP_SMALL_CONST, code + 1);
      fmtOp(text, (code[0] >> 6) & 1 ? OP_VARIABLE : OP_SMALL_CONST, code + 2);
      return 3;
   }

   unsigned disOp2_var(std::string& text, const uint8_t* code)
   {
      text = "2OP-";
      fmtHex(text, code[0] & 0x1F, 2);
      text += " ";
      return 1 + fmtOperands(text, 4, code + 1);
   }

   unsigned disOpV(std::string& text, const uint8_t* code)
   {
      text = "VAR-";
      fmtHex(text, code[0] & 0x1F, 2);
      text += " ";
      return 1 + fmtOperands(text,
                             ((code[0] == 0xEC) || (code[0] == 0xFA)) ? 8 : 4,
                             code + 1);
   }

   unsigned disOpE(std::string& text, const uint8_t* code)
   {
      text = "EXT-";
      fmtHex(text, code[1] & 0x1F, 2);
      text += " ";
      return 2 + fmtOperands(text, 4, code + 2);
   }

   // Not entirely sure why I needed to write this!
   template <typename TYPE>
   static void fmtHex(std::string& text, TYPE value, size_t digits = 0)
   {
      static const char pad = '0';

      for(signed n = sizeof(TYPE) * 2 - 1; n >= 0; n--)
      {
         unsigned digit = (value >> (n * 4)) & 0xF;

         if ((n != 0) && (digit == 0))
         {
            if ((value >> (n * 4)) != 0)
            {
               text += '0';
            }
            else if (n < digits)
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

} // namespace Z

#endif
