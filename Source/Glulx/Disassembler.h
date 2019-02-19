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

#ifndef GLULX_DISASSEMBLER_H
#define GLULX_DISASSEMBLER_H

#include <cstdint>
#include <cstring>
#include <string>

namespace Glulx {

//! Glulx disassembler
class Disassembler
{
public:
   Disassembler()
   {
      declOp(0x00, "nop",       "");

      declOp(0x10, "add",       "LLS");
      declOp(0x11, "sub",       "LLS");
      declOp(0x12, "mul",       "LLS");
      declOp(0x13, "div",       "LLS");
      declOp(0x14, "mod",       "LLS");
      declOp(0x15, "neg",       "LS");

      declOp(0x18, "bitand",    "LLS");
      declOp(0x19, "bitor",     "LLS");
      declOp(0x1A, "bitxor",    "LLS");
      declOp(0x1B, "bitnot",    "LS");
      declOp(0x1A, "shiftl",    "LLS");
      declOp(0x1A, "sshiftr",   "LLS");
      declOp(0x1A, "ushiftr",   "LLS");

      declOp(0x20, "jump",      "L");

      declOp(0x22, "jz",        "LL");
      declOp(0x23, "jnz",       "LL");
      declOp(0x24, "jeq",       "LLL");
      declOp(0x25, "jne",       "LLL");
      declOp(0x26, "jlt",       "LLL");
      declOp(0x27, "jge",       "LLL");
      declOp(0x28, "jgt",       "LLL");
      declOp(0x29, "jle",       "LLL");
      declOp(0x2A, "jltu",      "LLL");
      declOp(0x2B, "jgeu",      "LLL");
      declOp(0x2C, "jgtu",      "LLL");
      declOp(0x2D, "jleu",      "LLL");

      declOp(0x30, "call",      "LLL");
      declOp(0x31, "return",    "L");
      declOp(0x32, "catch",     "SL");
      declOp(0x33, "throw",     "LL");
      declOp(0x34, "tailcall",  "LL");

      declOp(0x40, "copy",      "LS");
      declOp(0x41, "copys",     "LS");
      declOp(0x42, "copyb",     "LS");

      declOp(0x44, "sexs",      "LS");
      declOp(0x45, "sexb",      "LS");

      declOp(0x48, "aload",     "LLS");
      declOp(0x49, "aloads",    "LLS");
      declOp(0x4A, "aloadb",    "LLS");
      declOp(0x4B, "aloadbit",  "LLS");

      declOp(0x4C, "astore",    "LLL");
      declOp(0x4D, "astores",   "LLL");
      declOp(0x4E, "astoreb",   "LLL");
      declOp(0x4F, "astorebit", "LLL");

      declOp(0x50, "stkcount",  "S");
      declOp(0x51, "stkpeek",   "LS");
      declOp(0x52, "stkswap",   "");
      declOp(0x53, "stkroll",   "LL");
      declOp(0x54, "stkcopy",   "L");

      declOp(0x70, "streamchr", "L");
      declOp(0x71, "streamnum", "L");
      declOp(0x72, "streamstr", "L");
      declOp(0x73, "streamuch", "L");

      declOp(0x100, "gestalt",    "LLS");
      declOp(0x101, "debugtrap",  "");
      declOp(0x102, "getmemsize", "S");
      declOp(0x103, "setmemsize", "L");
      declOp(0x104, "jumpabs",    "L");

      declOp(0x110, "random",    "S");
      declOp(0x111, "setrandom", "L");

      declOp(0x120, "quit",        "");
      declOp(0x121, "verify",      "S");
      declOp(0x122, "restart",     "");
      declOp(0x123, "save",        "LS");
      declOp(0x124, "restore",     "LS");
      declOp(0x125, "saveundo",    "S");
      declOp(0x126, "restoreunde", "S");
      declOp(0x127, "protect",     "LL");

      declOp(0x130, "glk",         "LLS");

      declOp(0x140, "getstrtbl",   "S");
      declOp(0x141, "setstrtbl",   "L");

      declOp(0x148, "getiosys",    "S");
      declOp(0x149, "setiosys",    "L");
      declOp(0x150, "linsearch",   "LLLLLLLS");
      declOp(0x151, "binsearch",   "LLLLLLLS");
      declOp(0x152, "linksearch",  "LLLLLLS");
      declOp(0x160, "callf",       "LS");
      declOp(0x161, "callfi",      "LLS");
      declOp(0x162, "callfii",     "LLLS");
      declOp(0x163, "callfiii",   "LLLLS");

      declOp(0x170, "mzero",  "LL");
      declOp(0x171, "mcopy",  "LLL");
      declOp(0x172, "malloc", "LS");
      declOp(0x173, "mfree",  "L");

      declOp(0x180, "accelfunc",  "LL");
      declOp(0x181, "accelparam", "LL");

      declOp(0x190, "numtof",  "LS");
      declOp(0x191, "ftonumz", "LS");
      declOp(0x192, "ftonumn", "LS");

      declOp(0x198, "ceil", "LS");
      declOp(0x199, "floor", "LS");
 
      declOp(0x1A0, "fadd",  "LLS");
      declOp(0x1A1, "fsub",  "LLS");
      declOp(0x1A2, "fmul",  "LLS");
      declOp(0x1A3, "fdiv",  "LLS");
      declOp(0x1A4, "fmod",  "LLS");

      declOp(0x1A8, "sqrt",  "LS");
      declOp(0x1A9, "exp",   "LS");
      declOp(0x1AA, "log",   "LS");
      declOp(0x1AB, "pow",   "LS");

      declOp(0x1B0, "sin",   "LS");
      declOp(0x1B1, "cos",   "LS");
      declOp(0x1B2, "tan",   "LS");
      declOp(0x1B3, "asin",  "LS");
      declOp(0x1B4, "acos",  "LS");
      declOp(0x1B5, "atan",  "LS");
      declOp(0x1B6, "atan2", "LLS");

      declOp(0x1C0, "jfeq", "LLL");
      declOp(0x1C1, "jfne", "LLL");
      declOp(0x1C2, "jflt", "LLL");
      declOp(0x1C3, "jfle", "LLL");
      declOp(0x1C4, "jfgt", "LLL");
      declOp(0x1C5, "jfge", "LLL");

      declOp(0x1C8, "jisnan", "LL");
      declOp(0x1C9, "jisinf", "LL");
   }

   //! Disassemble a single instruction
   unsigned disassemble(std::string& text, uint32_t inst_addr, uint8_t* raw)
   {
      text = "";

      fmtHex(text, inst_addr, 6);
      text += ": ";

      // Extract op-code
      uint32_t code = *raw++;
      if (code >= 0xC0)
      {
         code = (code & 0x0F) << 24;
         code |= uint32_t(*raw++) << 16;
         code |= uint32_t(*raw++) << 8;
         code |= uint32_t(*raw++);
         fmtHex(text, code, 7);
      }
      else if (code >= 0x80)
      {
         code = (code & 0x3F) << 8;
         code |= uint32_t(*raw++);
         fmtHex(text, code, 4);
      }
      else
      {
         text += "  ";
         fmtHex(text, code, 2);
      }

      text += "  ";

      if (code < MAX_OP_CODE)
      {
         text += op[code].mnemonic;

         for(size_t i=0; i<=max_mnemonic_len - strlen(op[code].mnemonic); i++)
         {
            text += " ";
         }

         uint8_t mode[MAX_OPERAND];

         // Fetch and split address mode nibbles
         for(unsigned i=0; i<op[code].num_operand; i += 2)
         {
            uint8_t mode_pair = *raw++;
            mode[i]   = mode_pair & 0xF;
            mode[i+1] = mode_pair >> 4;
         }

         // Pre-fetch operand addresses based on the LS 2 bits of the address mode
         for(unsigned i=0; i<op[code].num_operand; i++)
         {
            uint32_t addr{};

            switch(mode[i] & 0b11)
            {
            case 1:
               addr = *raw++;
               break;

            case 2:
               addr =  (*raw++) << 8;
               addr |= *raw++;
               break;

            case 3:
               addr =  (*raw++) << 24;
               addr |= (*raw++) << 16;
               addr |= (*raw++) << 8;
               addr |= *raw++;
               break;
            }

            if (!op[code].isLoad(i))
            {
               text += "[";
            }

            switch(mode[i])
            {
            case 0x0: text +="#0"; break;

            case 0x1:
               text += "#";
               addr = int32_t(addr << 24) >> 24;
               fmtHex(text, addr, 2);
               break;

            case 0x2:
               text += "#";
               addr = int32_t(addr << 16) >> 16;
               fmtHex(text, addr, 4);
               break;

            case 0x3:
               text += "#";
               fmtHex(text, addr);
               break;

            case 0x4: text += "X"; break;

            case 0x5:
            case 0x6:
            case 0x7:
               fmtHex(text, addr);
               break;

            case 0x8: text += "sp"; break;

            case 0x9:
            case 0xA:
            case 0xB:
               text += "local+";
               fmtHex(text, addr);
               break;

            case 0xC: text += "X"; break;

            case 0xD:
            case 0xE:
            case 0xF:
               text += "ram+";
               fmtHex(text, addr);
               break;
            }

            if (!op[code].isLoad(i))
            {
               text += "]";
            }

            if ((i + 1) != op[code].num_operand) text += ", ";
         }
      }

      return 0;
   }

private:
   static const unsigned MAX_OPERAND = 8;
   static const unsigned MAX_OP_CODE = 0x1CA;

   class Op
   {
   public:
      const char* mnemonic{""};
      uint8_t     num_operand{0};
      uint8_t     operand_type{0};

      bool isLoad(unsigned i) const
      {
         return (operand_type & 0b10000000 >> i) != 0;
      }

      void init(const char* mnemonic_, const char* operands)
      {
         mnemonic    = mnemonic_;
         num_operand = strlen(operands);

         for(unsigned i=0; operands[i] != '\0'; i++)
         {
            if (operands[i] == 'L')
            {
               setIsLoad(i);
            }
         }
      }

   private:
       void setIsLoad(unsigned i)
       {
          operand_type |= 0b10000000 >> i;
       }
   };

   Op     op[MAX_OP_CODE];
   size_t max_mnemonic_len{0};

   //! Declare an operand
   void declOp(uint32_t code, const char* mnemonic, const char* operands)
   {
      op[code].init(mnemonic, operands);

      size_t len = strlen(mnemonic);
      if (len > max_mnemonic_len)
      {
         max_mnemonic_len = len;
      }
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

} // namespace Glulx

#endif
