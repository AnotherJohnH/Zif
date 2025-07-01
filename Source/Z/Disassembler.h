//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

#include "common/Disassembler.h"

namespace Z {

enum OperandType : uint8_t
{
   OP_LARGE_CONST = 0,
   OP_SMALL_CONST = 1,
   OP_VARIABLE    = 2,
   OP_NONE        = 3
};

//! Z Disassembler
class Disassembler : public IF::Disassembler
{
public:
   Disassembler(unsigned version)
   {
      // Ops supported on all versions
      declOp( 0x0, '0', 'S', "rtrue");
      declOp( 0x1, '0', 'S', "rfalse");
      declOp( 0x2, '0', 'l', "print");
      declOp( 0x3, '0', 'l', "print_ret");
      declOp( 0x4, '0', '_', "nop");
      declOp( 0x7, '0', '_', "restart");
      declOp( 0x8, '0', '_', "ret_popped");
      declOp( 0xA, '0', '_', "quit");
      declOp( 0xB, '0', '_', "new_line");

      declOp( 0x0, '1', 'B', "jz");
      declOp( 0x1, '1', '_', "get_sibling");
      declOp( 0x2, '1', '_', "get_child");
      declOp( 0x3, '1', 'S', "get_parent");
      declOp( 0x4, '1', 'S', "get_prop_len");
      declOp( 0x5, '1', '_', "inc");
      declOp( 0x6, '1', '_', "dec");
      declOp( 0x7, '1', '_', "print_addr");
      declOp( 0x9, '1', '_', "remove_obj");
      declOp( 0xA, '1', '_', "print_obj");
      declOp( 0xB, '1', '_', "ret");
      declOp( 0xC, '1', '_', "jump");
      declOp( 0xD, '1', '_', "print_paddr");
      declOp( 0xE, '1', 'S', "load");

      declOp(0x01, '2', 'B', "je");
      declOp(0x02, '2', 'B', "jl");
      declOp(0x03, '2', 'B', "jg");
      declOp(0x04, '2', 'B', "dec_chk");
      declOp(0x05, '2', 'B', "inc_chk");
      declOp(0x06, '2', 'B', "jin");
      declOp(0x07, '2', 'B', "test_bitmap");
      declOp(0x08, '2', 'S', "or");
      declOp(0x09, '2', 'S', "and");
      declOp(0x0A, '2', 'B', "test_attr");
      declOp(0x0B, '2', '_', "set_attr");
      declOp(0x0C, '2', '_', "clear_attr");
      declOp(0x0D, '2', '_', "store");
      declOp(0x0E, '2', '_', "insert_obj");
      declOp(0x0F, '2', 'S', "loadw");
      declOp(0x10, '2', 'S', "loadb");
      declOp(0x11, '2', 'S', "get_prop");
      declOp(0x12, '2', 'S', "get_prop_addr");
      declOp(0x13, '2', 'S', "get_naext_prop");
      declOp(0x14, '2', 'S', "add");
      declOp(0x15, '2', 'S', "sub");
      declOp(0x16, '2', 'S', "mul");
      declOp(0x17, '2', 'S', "div");
      declOp(0x18, '2', 'S', "mod");

      declOp(0x01, 'V', '_', "storew");
      declOp(0x02, 'V', '_', "storeb");
      declOp(0x03, 'V', '_', "put_prop");
      declOp(0x05, 'V', '_', "print_char");
      declOp(0x06, 'V', '_', "print_num");
      declOp(0x07, 'V', 'S', "random");
      declOp(0x08, 'V', '_', "push");
      declOp(0x09, 'V', '_', "pull");

      // Version specific ops
      if (version <= 3)
      {
         declOp(0x00, 'V', '_', "call");
      }

      if (version == 3)
      {
         declOp(0xC, '0', '_', "show_status");
      }

      if (version >= 3)
      {
         declOp(0xD,  '0', 'B', "verify");

         declOp(0x0A, 'V', '_', "split_window");
         declOp(0x0B, 'V', '_', "set_window");
         declOp(0x13, 'V', '_', "output_stream");
         declOp(0x14, 'V', '_', "input_stream");
      }

      if (version <= 4)
      {
         declOp(0x5,  '0', 'B', "save");
         declOp(0x6,  '0', 'B', "restore");
         declOp(0x9,  '0', '_', "pop");

         declOp(0xF,  '1', 'S', "not");

         declOp(0x04, 'V', '_', "sread");
      }

      if (version >= 4)
      {
         declOp( 0xC, '0', '_', "nop");

         declOp( 0x8, '1', 'S', "call_1s");
         declOp( 0xF, '1', '_', "call_1n");

         declOp(0x19, '2', '_', "call_2s");

         declOp(0x00, 'V', '_', "call_vs");
         declOp(0x0C, 'V', '_', "call_vs2");
         declOp(0x0D, 'V', '_', "erase_window");
         declOp(0x0E, 'V', '_', "erase_line");
         declOp(0x0F, 'V', '_', "set_cursor");
         declOp(0x10, 'V', '_', "get_cursor");
         declOp(0x11, 'V', '_', "set_text_style");
         declOp(0x12, 'V', '_', "buffer_mode");
         declOp(0x16, 'V', '_', "read_char");
         declOp(0x17, 'V', '_', "scan_table");
      }

      if (version >= 5)
      {
         declOp( 0x9, '0', 'S', "catch");
         declOp( 0xF, '0', 'B', "piracy");

         declOp(0x1A, '2', '_', "call_2n");
         declOp(0x1B, '2', '_', "set_colour");
         declOp(0x1C, '2', '_', "throw");

         declOp(0x04, 'V', '_', "aread");
         declOp(0x15, 'V', '_', "sound_effect");
         declOp(0x18, 'V', '_', "not");
         declOp(0x19, 'V', '_', "call_vn");
         declOp(0x1A, 'V', '_', "call_vn2");
         declOp(0x1B, 'V', '_', "tokenise");
         declOp(0x1C, 'V', '_', "encode_text");
         declOp(0x1D, 'V', '_', "copy_table");
         declOp(0x1E, 'V', '_', "print_table");
         declOp(0x1F, 'V', '_', "check_arg_count");

         declOp(0x00, 'E', 'S', "save_table");
         declOp(0x01, 'E', 'S', "restore_table");
         declOp(0x02, 'E', 'S', "log_shift");
         declOp(0x03, 'E', 'S', "art_shift");
         declOp(0x04, 'E', 'S', "set_font");
         declOp(0x09, 'E', 'S', "save_undo");
         declOp(0x0A, 'E', 'S', "restore_undo");
         declOp(0x0B, 'E', '_', "print_unicode");
         declOp(0x0C, 'E', '_', "check_unicode");
      }

      if (version == 6)
      {
         declOp(0x05, 'E', '_', "draw_picture");
         declOp(0x06, 'E', 'B', "picture_data");
         declOp(0x07, 'E', '_', "erase_picture");
         declOp(0x08, 'E', '_', "set_margin");
         declOp(0x10, 'E', '_', "move_window");
         declOp(0x11, 'E', '_', "window_size");
         declOp(0x12, 'E', '_', "window_style");
         declOp(0x13, 'E', 'S', "get_wind_prop");
         declOp(0x14, 'E', '_', "scroll_window");
         declOp(0x15, 'E', '_', "pop_stack");
         declOp(0x16, 'E', '_', "read_mouse");
         declOp(0x17, 'E', '_', "mouse_window");
         declOp(0x18, 'E', 'B', "push_stack");
         declOp(0x19, 'E', '_', "put_wind_prop");
         declOp(0x1A, 'E', '_', "print_form");
         declOp(0x1B, 'E', 'B', "make_menu");
         declOp(0x1C, 'E', '_', "picture_table");
      }
   }

private:
   struct Op
   {
      const char* mnemonic{""};
      char        in_type{'\0'};
      char        out_type{'\0'};
      uint8_t     variant{0};

      bool isInitialised() const { return in_type != '\0'; }

      void init(char in_type_, char out_type_, const char* mnemonic_, uint8_t variant_ = 0)
      {
         assert(!isInitialised() && (in_type_ != '\0'));

         mnemonic = mnemonic_;
         in_type  = in_type_;
         out_type = out_type_;
         variant  = variant_;
      }
   };

   Op     op[0x100];
   Op     opE[0x20];
   size_t max_mnemonic_len{0};

   //! Declare op
   void declOp(uint8_t code, char in, char out, const char* mnemonic)
   {
      size_t len = strlen(mnemonic);
      if (len > max_mnemonic_len)
      {
         max_mnemonic_len = len;
      }

      switch(in)
      {
      case '0':
         op[0xB0 + code].init(in, out, mnemonic); // B0..BF
         break;

      case '1':
         op[0x80 + code].init(in, out, mnemonic, 0); // 80..8F
         op[0x90 + code].init(in, out, mnemonic, 1); // 90..9F
         op[0xA0 + code].init(in, out, mnemonic, 2); // A0..AF
         break;

      case '2':
         op[0x00 + code].init(in,  out, mnemonic, 0b00); // 00..1F
         op[0x20 + code].init(in,  out, mnemonic, 0b01); // 20..3F
         op[0x40 + code].init(in,  out, mnemonic, 0b10); // 40..5F
         op[0x60 + code].init(in,  out, mnemonic, 0b11); // 60..7F
         op[0xC0 + code].init('V', out, mnemonic, 4); // C0..DF
         break;

      case 'V':
         op[0xE0 + code].init(in, out, mnemonic,
                              ((code == 0xC) || (code == 0x1A)) ? 8 : 4); // E0..FF
         break;

      case 'E':
         opE[code].init('V', out, mnemonic, 4); // 00..1F
         break;
      }
   }

   //!
   void fmtVar(std::string& text, uint8_t index) const
   {
      if (index == 0)
      {
         text += " sp";
      }
      else if (index < 16)
      {
         text += " fp+";
         text += std::to_string(index);
      }
      else
      {
         text += " g";
         text += std::to_string(index - 16);
      }
   }

   //! Disassemble a single operand
   unsigned fmtOp(std::string& text, unsigned op_type, const uint8_t* code) const
   {
      switch(op_type)
      {
      case OP_LARGE_CONST:
         text += " $";
         {
            uint16_t value = (uint16_t(code[0])<<8) | code[1];
            addHexString(text, value);
         }
         return 2;

      case OP_SMALL_CONST:
         text += " $";
         addHexString(text, code[0]);
         return 1;

      case OP_VARIABLE:
         fmtVar(text, code[0]);
         return 1;

      case OP_NONE:
         return 0;
      }
 
      assert(!"bad operand type");

      return 0;
   }

   //! Disassemble a variable number of operands
   unsigned fmtVarOperands(std::string& text, unsigned n, const uint8_t* code) const
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

   //! Decode a single op
   virtual unsigned decodeOp(std::string& text, const uint8_t* raw, bool pack) const override
   {
      unsigned  n = 0;

      uint8_t   code   = raw[n++];
      const Op* decode = &op[code];

      if (code == 0xBE)
      {
          // Extended operation
          code = raw[n++];
          decode = &opE[code & 0x1F];
      }

      text = decode->mnemonic;

      if (pack)
      {
         text += " ";
      }
      else
      {
         for(size_t i=0; i<=max_mnemonic_len - strlen(decode->mnemonic); i++)
         {
            text += " ";
         }
      }

      switch(decode->in_type)
      {
      case '0':
         // No input operands
         break;

      case '1':
         // Single input operand
         n += fmtOp(text, decode->variant, raw + n);
         break;

      case '2':
         // Two input operands
         n += fmtOp(text, decode->variant & 0b10 ? OP_VARIABLE : OP_SMALL_CONST, raw + n);
         n += fmtOp(text, decode->variant & 0b01 ? OP_VARIABLE : OP_SMALL_CONST, raw + n);
         break;

      case 'V':
         // Variable number of input operands
         n += fmtVarOperands(text, decode->variant, raw + n);
         break;

      default:
         assert(!"bad input operand type");
         break;
      }

      switch(decode->out_type)
      {
      case '_':
         // No output operand
         break;

      case 'l':
         text += " literal-string...";
         break;

      case 'S':
         // Result
         text += " ->";
         fmtVar(text, raw[n++]);
         break;

      case 'B':
         {
            // Label
            uint8_t type        = raw[n++];
            bool    if_true     = (type & 0b10000000) != 0;
            bool    long_branch = (type & 0b01000000) == 0;
            int16_t offset      =  type & 0b00111111;

            text += if_true ? " ?T " : " ?F ";

            if (long_branch)
            {
               offset = (offset << 8) | raw[n++];
            }

            if (offset == 0)
            {
               text += "false";
            }
            else if (offset == 1)
            {
               text += "true";
            }
            else
            {
               text += std::to_string(offset);
            }
         }
         break;

      default:
         assert(!"bad input operand type");
         break;
      }

      return n;
   }
};

} // namespace Z

