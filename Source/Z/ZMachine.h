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

#ifndef ZMACHINE_H
#define ZMACHINE_H

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "Options.h"
#include "Log.h"

#include "ZConfig.h"
#include "ZHeader.h"
#include "ZBlorb.h"
#include "ZObject.h"
#include "ZParser.h"
#include "ZState.h"
#include "ZText.h"
#include "ZWindowManager.h"


#define TRACE if (1) ; else trace.printf


//! Z machine implementation
class ZMachine : public ZState
{
private:
   typedef void (ZMachine::*OpPtr)();

   static const unsigned MAX_OPERANDS = 8;

   enum OperandType : uint8_t
   {
      OP_LARGE_CONST = 0,
      OP_SMALL_CONST = 1,
      OP_VARIABLE    = 2,
      OP_NONE        = 3
   };

   Log            trace{"trace.log"};
   Options&       options;
   Console&       console;
   ZStream        stream;
   ZWindowManager window_mgr;
   ZObject        object;
   ZText          text;
   ZParser        parser;
   ZBlorb         zblorb{};
   ZHeader*       header{};
   const char*    filename{};
   const char*    story{};
   unsigned       file_offset{0};
   uint32_t       inst_addr;
   unsigned       num_arg;
   union
   {
      uint16_t uarg[MAX_OPERANDS];
      int16_t  sarg[MAX_OPERANDS];
   };

   unsigned undo_index{0};

   // Op-code decoders
   OpPtr op0[0x10];
   OpPtr op1[0x10];
   OpPtr op2[0x20];
   OpPtr opV[0x20];
   OpPtr opE[0x20];

   void info(const char* format, ...)
   {
      va_list ap;
      va_start(ap, format);
      stream.vmessage(ZStream::INFO, format, ap);
      va_end(ap);
   }

   void warning(const char* format, ...)
   {
      va_list ap;
      va_start(ap, format);
      stream.vmessage(ZStream::WARNING, format, ap);
      va_end(ap);
   }

   void error(const char* format, ...)
   {
      va_list ap;
      va_start(ap, format);
      stream.vmessage(ZStream::ERROR, format, ap);
      va_end(ap);

      ZState::quit();
   }

   unsigned version() const { return header->version; }


   //! Conditional branch (4.7)
   void branch(bool cond)
   {
      uint8_t type           = ZState::fetchByte();
      bool    branch_if_true = (type & (1 << 7)) != 0;
      bool    long_branch    = (type & (1 << 6)) == 0;
      int16_t offset         = type & 0x3F;

      if(long_branch)
      {
         offset = (offset << 8) | fetchByte();
         // Sign extend
         offset = int16_t(offset << 2) >> 2;
      }

      TRACE(" B%c %04X", branch_if_true ? 'T' : 'F', offset & 0xFFFF);

      if(cond == branch_if_true)
      {
         if((offset == 0) || (offset == 1))
         {
            // return false or true from the current routine (4.7.1)
            subRet(offset);
         }
         else
         {
            // branch (4.7.2)
            ZState::branch(offset - 2);
         }
      }
   }

   //! Call a sub-routine
   void subCall(uint16_t        call_type,
                uint16_t        packed_addr,
                uint16_t        argc,
                const uint16_t* argv)
   {
      uint32_t target = header->unpackAddr(packed_addr, /* routine */ true);
      if (target == 0)
      {
         // this is legal, just return false
         switch(call_type)
         {
         case 0:  varWrite(fetchByte(), 0); break;
         case 1:  /* throw return value away */ break;
         case 2:  ZState::push(0); break;
         default: ZState::error(Error::BAD_CALL_TYPE); break;
         }
         return;
      }

      ZState::call(call_type, target);

      uint8_t num_locals = fetchByte();

      ZState::push(argc);

      for(unsigned i = 0; i < num_locals; ++i)
      {
         uint16_t value = 0;

         if(version() <= 4)
         {
            value = fetchWord();
         }

         if(i < argc)
         {
            value = argv[i];
         }

         ZState::push(value);
      }

      TRACE("   // call %06x args=%d locals=%d",
            ZState::getPC(), argc, num_locals - argc);
   }

   //! Return from a sub-routine
   void subRet(uint16_t value, uint16_t frame_ptr = 0xFFFF)
   {
      if(frame_ptr == 0xFFFF)
      {
         frame_ptr = ZState::getFramePtr();
      }

      uint16_t call_type = ZState::returnFromFrame(frame_ptr);

      switch(call_type)
      {
      case 0: varWrite(fetchByte(), value); break;
      case 1: /* throw return value away */ break;
      case 2: ZState::push(value);          break;

      default: ZState::error(Error::BAD_CALL_TYPE);
      }
   }

   bool readChar(uint16_t timeout, uint16_t routine, uint16_t& ch)
   {
      if(!stream.readChar(ch, timeout))
      {
         if(routine == 0) return false;

         subCall(0, routine, 0, 0);

         return false;
      }

      return true;
   }

   void showStatus() { TODO_WARN("show_status"); }

   void ILLEGAL() { ZState::error(Error::ILLEGAL_OP); }

   void TODO_ERROR(const char* op) { error(op); }
   void TODO_WARN(const char* op) { warning(op); }

   //============================================================================
   // Zero operand instructions

   //! rtrue - Return true (i.e. 1) from the current routine
   void op0_rtrue() { subRet(1); }

   //! rfalse - Return false (i.e. 0) from the current routine
   void op0_rfalse() { subRet(0); }

   //! print - Print the literal Z-encoded string
   void op0_print() { ZState::jump(text.print(ZState::getPC())); }

   //! print_ret - Print the literal Z-encoded string, a new-line then return true
   void op0_print_ret()
   {
      op0_print();
      op0_new_line();
      subRet(1);
   }

   //! nop - Probably the offiical "nop"
   void op0_nop() {}

   //! v1 save ?(label)
   void op0_save_v1() { branch(ZState::save(options.save_dir, story)); }

   //! v4 save -> (result)
   void op0_save_v4()
   {
      uint8_t ret = fetchByte();
      varWrite(ret, 2);
      varWrite(ret, ZState::save(options.save_dir, story) ? 1 : 0);
   }

   //! v1 restore ?(label)
   void op0_restore_v1() { branch(ZState::restore(options.save_dir, story)); }

   //! v4 restore -> (result)
   void op0_restore_v4()
   {
      if(!ZState::restore(options.save_dir, story)) varWrite(fetchByte(), 0);
   }

   //! restart
   void op0_restart() { start(/* restore_save */false); }

   //! ret_popped
   void op0_ret_popped() { subRet(ZState::pop()); }

   //! pop
   void op0_pop() { ZState::pop(); }

   //! catch -> (result)
   void op0_catch() { varWrite(fetchByte(), ZState::getFramePtr()); }

   //! quit
   void op0_quit() { ZState::quit(); }

   //! new_line
   void op0_new_line() { stream.writeChar('\n'); }

   //! show_status
   void op0_show_status() { showStatus(); }

   //! verify ?(label)
   void op0_verify() { branch(isChecksumOk()); }

   //! piracy ?(label)
   void op0_piracy() { branch(true); }

   //============================================================================
   // One operand instructions

   void op1_jz() { branch(uarg[0] == 0); }

   void op1_get_sibling()
   {
      uint16_t obj = object.getSibling(uarg[0]);
      varWrite(fetchByte(), obj);
      branch(obj != 0);
   }

   void op1_get_parent()
   {
      uint16_t obj = object.getParent(uarg[0]);
      varWrite(fetchByte(), obj);
   }

   void op1_get_child()
   {
      uint16_t obj = object.getChild(uarg[0]);
      varWrite(fetchByte(), obj);
      branch(obj != 0);
   }

   void op1_get_prop_len()  { varWrite(fetchByte(), object.propSize(uarg[0])); }

   void op1_inc()           { varWrite(uarg[0], varRead(uarg[0]) + 1); }

   void op1_dec()           { varWrite(uarg[0], varRead(uarg[0]) - 1); }

   void op1_print_addr()    { text.print(uarg[0]); }

   void op1_call_1s()       { subCall(0, uarg[0], 0, 0); }

   void op1_remove_obj()    { object.remove(uarg[0]); }

   void op1_print_obj()     { text.print(object.getName(uarg[0])); }

   void op1_ret()           { subRet(uarg[0]); }

   void op1_jump()          { ZState::branch(sarg[0] - 2); }

   void op1_print_paddr()   { text.print(header->unpackAddr(uarg[0], /* routine */false)); }

   void op1_load()          { varWrite(fetchByte(), varRead(uarg[0], true)); }

   void op1_not()           { varWrite(uarg[0], ~uarg[0]); }

   void op1_call_1n()       { subCall(1, uarg[0], 0, 0); }

   //============================================================================
   // Two operand instructions

   void op2_je()
   {
      branch(((num_arg > 1) && (uarg[0] == uarg[1])) ||
             ((num_arg > 2) && (uarg[0] == uarg[2])) ||
             ((num_arg > 3) && (uarg[0] == uarg[3])));
   }

   void op2_jl() { branch(sarg[0] < sarg[1]); }
   void op2_jg() { branch(sarg[0] > sarg[1]); }

   void op2_dec_chk()
   {
      int16_t value = varRead(uarg[0]) - 1;
      varWrite(uarg[0], value);
      branch(value < sarg[1]);
   }

   void op2_inc_chk()
   {
      int16_t value = varRead(uarg[0]) + 1;
      varWrite(uarg[0], value);
      branch(value > sarg[1]);
   }

   void op2_jin()           { branch(object.getParent(uarg[0]) == uarg[1]); }
   void op2_test_bitmap()   { branch((uarg[0] & uarg[1]) == uarg[1]); }
   void op2_or()            { varWrite(fetchByte(), uarg[0] | uarg[1]); }
   void op2_and()           { varWrite(fetchByte(), uarg[0] & uarg[1]); }
   void op2_test_attr()     { branch(object.getAttr(uarg[0], uarg[1])); }
   void op2_set_attr()      { object.setAttr(uarg[0], uarg[1], true); }
   void op2_clear_attr()    { object.setAttr(uarg[0], uarg[1], false); }
   void op2_store()         { varWrite(uarg[0], uarg[1], true); }
   void op2_insert_obj()    { object.insert(uarg[0], uarg[1]); }

   //! 2OP:15 0F loadw array word_index -> (result)
   void op2_loadw()
   {
      varWrite(fetchByte(), memory.readWord(uarg[0]+2*uarg[1]));
   }

   //! 2OP:16 10 loadb array byte_index -> (result)
   //  Stores array->byte_index (i.e., the byte at address array+byte_index,
   //  which must lie in static or dynamic memory)
   void op2_loadb()
   {
      varWrite(fetchByte(), memory[uarg[0] + uarg[1]]);
   }

   void op2_get_prop()      { varWrite(fetchByte(), object.getProp(uarg[0], uarg[1])); }
   void op2_get_prop_addr() { varWrite(fetchByte(), object.getPropAddr(uarg[0], uarg[1])); }
   void op2_get_next_prop() { varWrite(fetchByte(), object.getPropNext(uarg[0], uarg[1])); }
   void op2_add()           { varWrite(fetchByte(), sarg[0] + sarg[1]); }
   void op2_sub()           { varWrite(fetchByte(), sarg[0] - sarg[1]); }
   void op2_mul()           { varWrite(fetchByte(), sarg[0] * sarg[1]); }

   void op2_div()
   {
      if(sarg[1] == 0)
      {
         ZState::error(Error::DIV_BY_ZERO);
         return;
      }
      varWrite(fetchByte(), sarg[0] / sarg[1]);
   }

   void op2_mod()
   {
      if(sarg[1] == 0)
      {
         ZState::error(Error::DIV_BY_ZERO);
         return;
      }
      varWrite(fetchByte(), sarg[0] % sarg[1]);
   }

   void op2_call_2s()           { subCall(0, uarg[0], 1, &uarg[1]); }
   void op2_call_2n()           { subCall(1, uarg[0], 1, &uarg[1]); }
   void op2_set_colour()        { stream.setColours(uarg[0], uarg[1]); /* TODO v6 window */ }
   void op2_throw()             { subRet(uarg[0], uarg[1]); }

   //============================================================================
   // Variable operand instructions
   void opV_call()
   {
      if (uarg[0] == 0)
         varWrite(fetchByte(), 0);
      else
         subCall(0, uarg[0], num_arg-1, &uarg[1]);
   }

   void opV_call_vs()        { opV_call(); }
   void opV_not()            { varWrite(fetchByte(), ~uarg[0]); }
   void opV_call_vn()        { subCall(1, uarg[0], num_arg-1, &uarg[1]); }
   void opV_call_vn2()       { opV_call_vn(); }
   void opV_storew()         { memory.writeWord(uarg[0] + 2*uarg[1], uarg[2]); }
   void opV_storeb()         { memory[uarg[0] + uarg[1]] = uarg[2]; }
   void opV_put_prop()       { object.setProp(uarg[0], uarg[1], uarg[2]); }

   //! V1 sread text parse
   //! V4 sread text parse timeout routine
   template <bool TIMER, bool SHOW_STATUS>
   void opV_sread()
   {
      uint16_t buffer  = uarg[0];
      uint16_t parse   = uarg[1];
      uint16_t timeout = TIMER && (num_arg >= 3) ? uarg[2] : 0;
      uint16_t routine = TIMER && (num_arg >= 4) ? uarg[3] : 0;

      if(SHOW_STATUS) showStatus();

      uint8_t  max   = memory[buffer++] - 1;
      uint16_t start = buffer;

      for(uint8_t len = 0; len < max; len++)
      {
         uint16_t ch;

         if(!readChar(timeout, routine, ch))
         {
            return;
         }

         if(ch == '\b')
         {
            // => delete
            if(buffer > start)
            {
               stream.writeRaw(" \b");
               --buffer;
               --len;
            }
         }
         else if(ch == '\n')
         {
            memory[buffer] = '\0';
            break;
         }
         else
         {
            memory[buffer++] = tolower(ch);
         }
      }

      parser.tokenise(memory, parse, start, header->dict, false);
   }

   //! aread text parse timeout routine -> (result)
   void opV_aread()
   {
      uint16_t buffer  = uarg[0];
      uint16_t parse   = uarg[1];
      uint16_t timeout = num_arg >= 3 ? uarg[2] : 0;
      uint16_t routine = num_arg >= 4 ? uarg[3] : 0;

      uint8_t max = memory[buffer++];
      uint8_t len = memory[buffer++];

      uint16_t start  = buffer;
      uint8_t  status = 0;

      for(; len < max; len++)
      {
         uint16_t ch;

         if(!readChar(timeout, routine, ch))
         {
            break;
         }

         if(ch == '\b')
         {
            // => delete
            if(buffer > start)
            {
               stream.writeRaw(" \b");
               --buffer;
               --len;
            }
         }
         else if(ch == '\n')
         {
            memory[buffer]    = '\0';
            memory[start - 1] = len;
            status = ch;
            break;
         }
         else
         {
            memory[buffer++] = ch;
         }
      }

      varWrite(fetchByte(), status);

      if(parse != 0)
      {
         parser.tokenise(memory, parse, start, header->dict, false);
      }
   }

   void opV_print_char()     { stream.writeChar(uarg[0]); }
   void opV_print_num()      { stream.writeNumber(sarg[0]); }
   void opV_random()         { varWrite(fetchByte(), random(sarg[0])); }
   void opV_push()           { ZState::push(uarg[0]); }

   void opV_pull_v1()
   {
      varWrite(uarg[0], ZState::pop(), true);
   }

   void opV_pull_v6()
   {
      uint16_t value;
      if(num_arg == 1)
      {
         // User stack
         uint16_t st  = uarg[0];
         uint16_t ptr = memory.readWord(st);
         memory.writeWord(st, ++ptr);
         value = memory.readWord(ptr + 2 * ptr);
      }
      else
      {
         value = ZState::pop();
      }

      varWrite(fetchByte(), value, true);
   }

   void opV_split_window()   { window_mgr.split(uarg[0]); }
   void opV_set_window()     { window_mgr.select(uarg[0]); }
   void opV_call_vs2()       { subCall(0, uarg[0], num_arg - 1, &uarg[1]); }
   void opV_erase_window()   { window_mgr.eraseWindow(uarg[0]); }

   void opV_erase_line_v4()
   {
      if (uarg[0] == 1)
         console.eraseLine();
   }

   void opV_erase_line_v6()
   {
      if (uarg[0] == 1)
         console.eraseLine();
      else
         TODO_WARN("v6 op erase_line pixels unimplemented");
   }

   void opV_set_cursor_v4()  { console.moveCursor(uarg[0], uarg[1]); }
   void opV_set_cursor_v6()  { TODO_WARN("op set_cursor_v6 unimplemented"); }

   void opV_get_cursor()
   {
      unsigned row, col;
      console.getCursorPos(row, col);

      uint16_t array = uarg[0];
      memory.writeWord(array + 0, row);
      memory.writeWord(array + 2, col);
   }

   void opV_set_text_style() { stream.setFontStyle(uarg[0]); }
   void opV_buffer_mode()    { stream.setBuffering(uarg[0] != 0); }

   void opV_output_stream()
   {
      int16_t number = sarg[0];

      if(number == 3)
      {
         uint32_t table = num_arg >= 2 ? uarg[1] : 0;
         int16_t  width = num_arg == 3 ? sarg[2] : 0;

         stream.enableMemoryStream(table, width);
      }
      else
      {
         bool set = number > 0;
         number   = abs(number);
         if(number > 4)
            ZState::error(Error::BAD_STREAM);
         else
            stream.enableStream(abs(number), set);
      }
   }

   void opV_input_stream() { TODO_ERROR("op input_stream unimplemented"); }

   void opV_sound_effect() { TODO_WARN("op sound_effect unimplemeneted"); }

   void opV_read_char()
   {
      // assert(uarg[0] == 1);
      uint16_t timeout = num_arg >= 2 ? uarg[1] : 0;
      uint16_t routine = num_arg >= 3 ? uarg[2] : 0;
      uint16_t ch;

      if(readChar(timeout, routine, ch))
      {
         varWrite(fetchByte(), ch);
      }
   }

   void opV_scan_table()
   {
      uint16_t x      = uarg[0];
      uint16_t table  = uarg[1];
      uint16_t len    = uarg[2];
      uint16_t form   = num_arg == 4 ? uarg[3] : 0x82;
      uint16_t result = 0;

      for(uint16_t i = 0; i < len; ++i)
      {
         uint16_t v = (form & 0x80) ? memory.readWord(table)
                                    : memory[table];

         if(v == x)
         {
            result = table;
            break;
         }

         table += form & 0x7F;
      }

      varWrite(fetchByte(), result);

      branch(result != 0);
   }

   void opV_tokenise()
   {
      uint16_t text  = uarg[0];
      uint16_t parse = uarg[1];
      uint16_t dict  = num_arg >= 3 ? uarg[2] : uint16_t(header->dict);
      bool     flag  = num_arg == 4 ? uarg[3] != 0 : false;

      parser.tokenise(memory, parse, text + 1, dict, flag);
   }

   void opV_encode_text() { TODO_ERROR("op encode_text unimplemeneted"); }

   void opV_copy_table()
   {
      uint16_t from = uarg[0];
      uint16_t to   = uarg[1];
      int16_t  size = sarg[2];

      if(to == 0)
      {
         memory.zero(from, from + size);
      }
      else if((size < 0) || (from > to))
      {
         memory.copyForward(from, to, abs(size));
      }
      else
      {
         memory.copyBackward(from, to, size);
      }
   }

   void opV_print_table()
   {
      uint16_t addr   = uarg[0];
      uint16_t width  = uarg[1];
      uint16_t height = num_arg >= 3 ? uarg[2] : 1;
      int16_t  skip   = num_arg == 4 ? uarg[3] : 0;

      text.printTable(addr, width, height, skip);
   }

   void opV_check_arg_count() { branch(uarg[0] <= ZState::getNumFrameArgs()); }

   //============================================================================
   // Extended operand instructions

   void opE_save_table()
   {
      uint16_t table = uarg[0];
      uint16_t bytes = uarg[1];
      uint16_t name  = uarg[2];

      (void)table;
      (void)bytes;
      (void)name; // TODO use supplied parameters

      uint8_t ret = fetchByte();
      varWrite(ret, 2);
      varWrite(ret, ZState::save(options.save_dir, story) ? 1 : 0);
   }

   void opE_restore_table()
   {
      uint16_t table = uarg[0];
      uint16_t bytes = uarg[1];
      uint16_t name  = uarg[2];

      (void)table;
      (void)bytes;
      (void)name; // TODO use supplied parameters

      if(!ZState::restore(options.save_dir, story)) varWrite(fetchByte(), 0);
   }

   void opE_log_shift()
   {
      if(sarg[1] < 0)
         varWrite(fetchByte(), uarg[0] >> -sarg[1]);
      else
         varWrite(fetchByte(), uarg[0] << sarg[1]);
   }

   void opE_art_shift()
   {
      if(sarg[1] < 0)
         varWrite(fetchByte(), sarg[0] >> -sarg[1]);
      else
         varWrite(fetchByte(), sarg[0] << sarg[1]);
   }

   void opE_save_undo()
   {
      char name[12];
      sprintf(name, "undo_%x", undo_index);

      undo_index = (undo_index + 1) % options.undo;

      uint8_t ret = fetchByte();
      varWrite(ret, 2);
      varWrite(ret, ZState::save(options.save_dir, story) ? 1 : 0);
   }

   void opE_restore_undo()
   {
      undo_index = undo_index == 0 ? options.undo - 1
                                   : undo_index - 1;

      char name[12];
      sprintf(name, "undo_%x", undo_index);

      if(!ZState::restore(options.save_dir, story)) varWrite(fetchByte(), 0);
   }

   void opE_print_unicode()
   {
      uint16_t ch = uarg[0];
      if (ch <= 126)
      {
         stream.writeChar(uarg[0]);
      }
      else
      {
         TODO_WARN("op print_unicode unimplemented");
      }
   }

   void opE_check_unicode()
   {
      uint16_t ch = uarg[0];
      varWrite(fetchByte(), ch <= 126);
   }

   void opE_draw_picture() { TODO_WARN("op draw_picture unimplemented"); }

   void opE_picture_data()
   {
      TODO_WARN("op picture_data unimplemented");

      uint16_t pict_no = uarg[0];
      uint16_t array   = uarg[1];
      bool     valid{false};
      uint16_t value1{0};
      uint16_t value2{0};

      if (pict_no == 0)
      {
         value1 = 0; // TODO number of available pictures
         value2 = 0; // TODO picture file release number
      }
      // TODO
      // else if (isValidPictur(pict_no))
      // {
      //    value1 = height;
      //    value2 = width;
      //    valid  = true;
      // }

      memory.writeWord(array + 0, value1);
      memory.writeWord(array + 2, value2);

      branch(valid);
   }

   void opE_erase_picture() { TODO_WARN("op erase_picture unimplemented"); }

   void opE_set_margins() { TODO_WARN("op set_margins unimplemented"); }

   //! EXT:4
   //  set_font font
   void opE_set_font()
   {
      bool ok = stream.setFont(uarg[0]);
      varWrite(fetchByte(), ok);
   }

   void opE_move_window()
   {
      uint16_t wind = uarg[0];
      uint16_t y    = uarg[1];
      uint16_t x    = uarg[2];

      (void)wind;
      (void)y;
      (void)x;

      TODO_WARN("move_window");
   }

   void opE_window_size()
   {
      uint16_t wind = uarg[0];
      uint16_t y    = uarg[1];
      uint16_t x    = uarg[2];

      (void)wind;
      (void)y;
      (void)x;

      TODO_WARN("window_size");
   }

   void opE_window_style()
   {
      uint16_t wind      = uarg[0];
      uint16_t flags     = uarg[1];
      uint16_t operation = uarg[2];

      (void)wind;
      (void)flags;
      (void)operation;

      TODO_WARN("set_wind_style");
   }

   void opE_get_wind_prop()
   {
      uint16_t wind = uarg[0];
      uint16_t prop = uarg[1];

      varWrite(fetchByte(), window_mgr.getWindowProp(wind, prop));
   }

   void opE_scroll_window() { TODO_WARN("scroll_window unimplemented"); }

   void opE_pop_stack() { TODO_ERROR("pop_stack unimplemented"); }

   void opE_read_mouse() { TODO_ERROR("read_mouse unimplemented"); }

   void opE_mouse_window() { TODO_WARN("mouse_window unimplemented"); }

   void opE_push_stack() { TODO_ERROR("push_stack unimplemented"); }

   void opE_put_wind_prop()
   {
      uint16_t wind  = uarg[0];
      uint16_t prop  = uarg[1];
      uint16_t value = uarg[2];

      window_mgr.setWindowProp(wind, prop, value);
   }

   void opE_print_form() { TODO_ERROR("print_form unimplemented"); }

   void opE_make_menu() { TODO_ERROR("make_menu unimplemented"); }

   void opE_picture_table() { TODO_WARN("picture_table unimplemented"); }

   void initDecoder()
   {
      // Zero operand instructions
      op0[0x0] =                  &ZMachine::op0_rtrue;
      op0[0x1] =                  &ZMachine::op0_rfalse;
      op0[0x2] =                  &ZMachine::op0_print;
      op0[0x3] =                  &ZMachine::op0_print_ret;
      op0[0x4] =                  &ZMachine::op0_nop;
      op0[0x5] = version() <= 3 ? &ZMachine::op0_save_v1
               : version() == 4 ? &ZMachine::op0_save_v4
                                : &ZMachine::ILLEGAL;
      op0[0x6] = version() <= 3 ? &ZMachine::op0_restore_v1
               : version() == 4 ? &ZMachine::op0_restore_v4
                                : &ZMachine::ILLEGAL;
      op0[0x7] =                  &ZMachine::op0_restart;
      op0[0x8] =                  &ZMachine::op0_ret_popped;
      op0[0x9] = version() <= 4 ? &ZMachine::op0_pop
                                : &ZMachine::op0_catch;
      op0[0xA] =                  &ZMachine::op0_quit;
      op0[0xB] =                  &ZMachine::op0_new_line;
      op0[0xC] = version() <= 2 ? &ZMachine::ILLEGAL
               : version() == 3 ? &ZMachine::op0_show_status
                                : &ZMachine::op0_nop;
      op0[0xD] = version() >= 3 ? &ZMachine::op0_verify
                                : &ZMachine::ILLEGAL;
      op0[0xE] =                  &ZMachine::ILLEGAL;   // "extend" decoded elsewhere
      op0[0xF] = version() >= 5 ? &ZMachine::op0_piracy
                                : &ZMachine::ILLEGAL;

      // One operand instructions
      op1[0x0] =                  &ZMachine::op1_jz;
      op1[0x1] =                  &ZMachine::op1_get_sibling;
      op1[0x2] =                  &ZMachine::op1_get_child;
      op1[0x3] =                  &ZMachine::op1_get_parent;
      op1[0x4] =                  &ZMachine::op1_get_prop_len;
      op1[0x5] =                  &ZMachine::op1_inc;
      op1[0x6] =                  &ZMachine::op1_dec;
      op1[0x7] =                  &ZMachine::op1_print_addr;
      op1[0x8] = version() >= 4 ? &ZMachine::op1_call_1s
                                : &ZMachine::ILLEGAL;
      op1[0x9] =                  &ZMachine::op1_remove_obj;
      op1[0xA] =                  &ZMachine::op1_print_obj;
      op1[0xB] =                  &ZMachine::op1_ret;
      op1[0xC] =                  &ZMachine::op1_jump;
      op1[0xD] =                  &ZMachine::op1_print_paddr;
      op1[0xE] =                  &ZMachine::op1_load;
      op1[0xF] = version() <= 4 ? &ZMachine::op1_not
                                : &ZMachine::op1_call_1n;

      // Two operand instructions
      op2[0x00] =                  &ZMachine::ILLEGAL;
      op2[0x01] =                  &ZMachine::op2_je;
      op2[0x02] =                  &ZMachine::op2_jl;
      op2[0x03] =                  &ZMachine::op2_jg;
      op2[0x04] =                  &ZMachine::op2_dec_chk;
      op2[0x05] =                  &ZMachine::op2_inc_chk;
      op2[0x06] =                  &ZMachine::op2_jin;
      op2[0x07] =                  &ZMachine::op2_test_bitmap;
      op2[0x08] =                  &ZMachine::op2_or;
      op2[0x09] =                  &ZMachine::op2_and;
      op2[0x0A] =                  &ZMachine::op2_test_attr;
      op2[0x0B] =                  &ZMachine::op2_set_attr;
      op2[0x0C] =                  &ZMachine::op2_clear_attr;
      op2[0x0D] =                  &ZMachine::op2_store;
      op2[0x0E] =                  &ZMachine::op2_insert_obj;
      op2[0x0F] =                  &ZMachine::op2_loadw;
      op2[0x10] =                  &ZMachine::op2_loadb;
      op2[0x11] =                  &ZMachine::op2_get_prop;
      op2[0x12] =                  &ZMachine::op2_get_prop_addr;
      op2[0x13] =                  &ZMachine::op2_get_next_prop;
      op2[0x14] =                  &ZMachine::op2_add;
      op2[0x15] =                  &ZMachine::op2_sub;
      op2[0x16] =                  &ZMachine::op2_mul;
      op2[0x17] =                  &ZMachine::op2_div;
      op2[0x18] =                  &ZMachine::op2_mod;
      op2[0x19] = version() >= 4 ? &ZMachine::op2_call_2s
                                 : &ZMachine::ILLEGAL;
      op2[0x1A] = version() >= 5 ? &ZMachine::op2_call_2n
                                 : &ZMachine::ILLEGAL;
      op2[0x1B] = version() >= 5 ? &ZMachine::op2_set_colour
                                 : &ZMachine::ILLEGAL;
      op2[0x1C] = version() >= 5 ? &ZMachine::op2_throw
                                 : &ZMachine::ILLEGAL;
      op2[0x1D] =                  &ZMachine::ILLEGAL;
      op2[0x1E] =                  &ZMachine::ILLEGAL;
      op2[0x1F] =                  &ZMachine::ILLEGAL;

      // Variable operand instructions
      opV[0x00] = version() <= 3 ? &ZMachine::opV_call
                                 : &ZMachine::opV_call_vs;
      opV[0x01] =                  &ZMachine::opV_storew;
      opV[0x02] =                  &ZMachine::opV_storeb;
      opV[0x03] =                  &ZMachine::opV_put_prop;
      opV[0x04] = version() <= 3 ? &ZMachine::opV_sread<false,true>
                : version() == 4 ? &ZMachine::opV_sread<true,false>
                                 : &ZMachine::opV_aread;
      opV[0x05] =                  &ZMachine::opV_print_char;
      opV[0x06] =                  &ZMachine::opV_print_num;
      opV[0x07] =                  &ZMachine::opV_random;
      opV[0x08] =                  &ZMachine::opV_push;
      opV[0x09] = version() == 6 ? &ZMachine::opV_pull_v6
                                 : &ZMachine::opV_pull_v1;
      opV[0x0A] = version() >= 3 ? &ZMachine::opV_split_window
                                 : &ZMachine::ILLEGAL;
      opV[0x0B] = version() >= 3 ? &ZMachine::opV_set_window
                                 : &ZMachine::ILLEGAL;
      opV[0x0C] = version() >= 4 ? &ZMachine::opV_call_vs2
                                 : &ZMachine::ILLEGAL;
      opV[0x0D] = version() >= 4 ? &ZMachine::opV_erase_window
                                 : &ZMachine::ILLEGAL;
      opV[0x0E] = version() >= 4 ? &ZMachine::opV_erase_line_v4
                : version() >= 6 ? &ZMachine::opV_erase_line_v6
                                 : &ZMachine::ILLEGAL;
      opV[0x0F] = version() >= 4 ? &ZMachine::opV_set_cursor_v4
                : version() >= 6 ? &ZMachine::opV_set_cursor_v6
                                 : &ZMachine::ILLEGAL;
      opV[0x10] = version() >= 4 ? &ZMachine::opV_get_cursor      : &ZMachine::ILLEGAL;
      opV[0x11] = version() >= 4 ? &ZMachine::opV_set_text_style  : &ZMachine::ILLEGAL;
      opV[0x12] = version() >= 4 ? &ZMachine::opV_buffer_mode     : &ZMachine::ILLEGAL;
      opV[0x13] = version() >= 3 ? &ZMachine::opV_output_stream   : &ZMachine::ILLEGAL;
      opV[0x14] = version() >= 3 ? &ZMachine::opV_input_stream    : &ZMachine::ILLEGAL;
      opV[0x15] = version() >= 5 ? &ZMachine::opV_sound_effect    : &ZMachine::ILLEGAL;
      opV[0x16] = version() >= 4 ? &ZMachine::opV_read_char       : &ZMachine::ILLEGAL;
      opV[0x17] = version() >= 4 ? &ZMachine::opV_scan_table      : &ZMachine::ILLEGAL;
      opV[0x18] = version() >= 5 ? &ZMachine::opV_not             : &ZMachine::ILLEGAL;
      opV[0x19] = version() >= 5 ? &ZMachine::opV_call_vn         : &ZMachine::ILLEGAL;
      opV[0x1A] = version() >= 5 ? &ZMachine::opV_call_vn2        : &ZMachine::ILLEGAL;
      opV[0x1B] = version() >= 5 ? &ZMachine::opV_tokenise        : &ZMachine::ILLEGAL;
      opV[0x1C] = version() >= 5 ? &ZMachine::opV_encode_text     : &ZMachine::ILLEGAL;
      opV[0x1D] = version() >= 5 ? &ZMachine::opV_copy_table      : &ZMachine::ILLEGAL;
      opV[0x1E] = version() >= 5 ? &ZMachine::opV_print_table     : &ZMachine::ILLEGAL;
      opV[0x1F] = version() >= 5 ? &ZMachine::opV_check_arg_count : &ZMachine::ILLEGAL;

      // Externded instructions
      for(unsigned i = 0; i <= 0x1F; i++)
      {
         opE[i] = &ZMachine::ILLEGAL;
      }

      if(version() < 5) return;

      opE[0x00] = &ZMachine::opE_save_table;
      opE[0x01] = &ZMachine::opE_restore_table;
      opE[0x02] = &ZMachine::opE_log_shift;
      opE[0x03] = &ZMachine::opE_art_shift;
      opE[0x04] = &ZMachine::opE_set_font;
      opE[0x09] = &ZMachine::opE_save_undo;
      opE[0x0A] = &ZMachine::opE_restore_undo;
      opE[0x0B] = &ZMachine::opE_print_unicode;
      opE[0x0C] = &ZMachine::opE_check_unicode;

      if(version() < 6) return;

      opE[0x05] = &ZMachine::opE_draw_picture;
      opE[0x06] = &ZMachine::opE_picture_data;
      opE[0x07] = &ZMachine::opE_erase_picture;
      opE[0x08] = &ZMachine::opE_set_margins;

      opE[0x10] = &ZMachine::opE_move_window;
      opE[0x11] = &ZMachine::opE_window_size;
      opE[0x12] = &ZMachine::opE_window_style;
      opE[0x13] = &ZMachine::opE_get_wind_prop;
      opE[0x14] = &ZMachine::opE_scroll_window;
      opE[0x15] = &ZMachine::opE_pop_stack;
      opE[0x16] = &ZMachine::opE_read_mouse;
      opE[0x17] = &ZMachine::opE_mouse_window;
      opE[0x18] = &ZMachine::opE_push_stack;
      opE[0x19] = &ZMachine::opE_put_wind_prop;
      opE[0x1A] = &ZMachine::opE_print_form;
      opE[0x1B] = &ZMachine::opE_make_menu;
      opE[0x1C] = &ZMachine::opE_picture_table;
   }

   //============================================================================

   void clearOperands() { num_arg = 0; }

   void fetchOperand(OperandType type)
   {
      uint16_t operand;

      switch(type)
      {
      case OP_LARGE_CONST: operand = fetchWord();          break;
      case OP_SMALL_CONST: operand = fetchByte();          break;
      case OP_VARIABLE:    operand = varRead(fetchByte()); break;
      default: assert(!"bad operand type"); return;
      }

      TRACE(" (%X)", operand);
      uarg[num_arg++] = operand;

      assert(num_arg <= 8);
   }

   void fetchOperands(unsigned n)
   {
      uint16_t op_types;

      if(n == 4)
      {
         op_types = fetchByte() << 8;
         TRACE(" t%02X", op_types >> 8);
      }
      else
      {
         assert(n == 8);

         op_types = fetchWord();
         TRACE(" t%04X", op_types);
      }

      // Unpack the type of the operands
      for(unsigned i = 0; i < n; ++i)
      {
         OperandType type = OperandType(op_types >> 14);

         if(type == OP_NONE) return;

         fetchOperand(type);

         op_types <<= 2;
      }
   }


   void doOp0(uint8_t op_code) { (this->*op0[op_code & 0xF])(); }

   void doOp1(uint8_t op_code)
   {
      fetchOperand(OperandType((op_code >> 4) & 3));

      (this->*op1[op_code & 0xF])();
   }

   void doOp2(uint8_t op_code)
   {
      fetchOperand(op_code & (1 << 6) ? OP_VARIABLE : OP_SMALL_CONST);
      fetchOperand(op_code & (1 << 5) ? OP_VARIABLE : OP_SMALL_CONST);

      (this->*op2[op_code & 0x1F])();
   }

   void doOp2_var(uint8_t op_code)
   {
      // TODO what if there are more than two arguments?
      fetchOperands(4);

      (this->*op2[op_code & 0x1F])();
   }

   void doOpV(uint8_t op_code)
   {
      if((op_code == 0xEC) || (op_code == 0xFA))
         fetchOperands(8);
      else
         fetchOperands(4);

      (this->*opV[op_code & 0x1F])();
   }

   void doOpE(uint8_t op_code)
   {
      fetchOperands(4);

      (this->*opE[op_code & 0x1F])();
   }


   void fetchDecodeExecute()
   {
      inst_addr = ZState::getPC();

      uint8_t opcode = fetchByte();

      clearOperands();

      if(opcode < 0x80)
      {
         // 0xxxxxx
         doOp2(opcode);
      }
      else if(opcode < 0xB0)
      {
         // 1000xxxx
         // 1001xxxx
         // 1010xxxx
         doOp1(opcode);
      }
      else if(opcode < 0xC0)
      {
         if(opcode == 0xBE)
         {
            // 10111110
            doOpE(fetchByte());
         }
         else
         {
            // 1011xxxx
            doOp0(opcode);
         }
      }
      else if(opcode < 0xE0)
      {
         // 110xxxxx
         doOp2_var(opcode);
      }
      else
      {
         // 111xxxxx
         doOpV(opcode);
      }
   }


   //! Reset machine state to intial conditions
   void start(bool restore_save)
   {
      console.clear();

      // TODO the header should be reset (only bits 0 and 1 from Flags 2
      //      shoud be preserved)

      if(!ZState::reset(filename, file_offset, header->getEntryPoint(), header->checksum))
      {
         error("Failed to read story z-file \"%s\"", filename);
      }

      if(!isChecksumOk())
      {
         warning("checksum fail");
      }

      if (restore_save)
      {
         ZState::restore(options.save_dir, story);
      }
   }

   bool loadHeader()
   {
      PLT::File file(nullptr, filename);

      if(!file.openForRead())
      {
         error("Failed to open story z-file \"%s\"", filename);
         return false;
      }

      file.seek(file_offset);

      // Read header
      if(!memory.load(file, 0, sizeof(ZHeader)))
      {
         error("Z-file header read failed");
         return false;
      }

      header = reinterpret_cast<ZHeader*>(&memory[0]);

      if(!header->isVersionValid())
      {
         error("Unexpected version %u", header->version);
         return false;
      }

      return true;
   }

   void disassemble(char* text)
   {
      // XXX this is all a bit unsafe and nasty - probably should
      // bite the bullet and alow dynamic allocation

      uint8_t opcode = memory[inst_addr];

      sprintf(text, "%06X: %02X ", inst_addr, opcode);
      text += strlen(text);

      if(opcode < 0x80)
      {
         sprintf(text, "   2OP:%02X", opcode & 0x1F);
      }
      else if(opcode < 0xB0)
      {
         sprintf(text, "   1OP:%1X", opcode & 0xF);
      }
      else if(opcode < 0xC0)
      {
         if((opcode & 0xF) == 0xE)
         {
            uint8_t ext_opcode = memory[inst_addr + 1];
            sprintf(text, "%02X EXT:%02X", ext_opcode, ext_opcode & 0x1F);
         }
         else
         {
            sprintf(text, "   0OP:%1X", opcode & 0xF);
         }
      }
      else if(opcode < 0xE0)
      {
         sprintf(text, "   2OP:%02X", opcode & 0x1F);
      }
      else
      {
         sprintf(text, "   VAR:%02X", opcode & 0x1F);
      }
   }

   void printTrace()
   {
      static unsigned tick = 0;

      inst_addr = ZState::getPC();

      char text[128];
      disassemble(text);

      trace.printf("%4d %s\n", tick++, text);
   }

public:
   ZMachine(Console& console_, Options& options_)
      : options(options_)
      , console(console_)
      , stream(console, options_, memory)
      , window_mgr(console, options_, stream)
      , object(memory)
      , text(stream, memory)
   {
   }

   //! Play a Z file.
   //! \return true if there were no errors
   bool play(const char* filename_, bool restore_save = false)
   {
      filename = filename_;

      story = strrchr(filename, '/');
      if (story == nullptr)
      {
         story = filename;
      }
      else
      {
         story = story + 1;
      }

      if (strstr(story, ".zblorb"))
      {
          if (!zblorb.findExecChunk(filename, "ZCOD", file_offset))
          {
             warning("ZCOD section not found in zblorb file");
             file_offset = 0;
          }
      }
      else
      {
          file_offset = 0;
      }

      if(!loadHeader()) return false;

      ZConfig config;
      config.interp_major_version = 1;
      config.interp_minor_version = 0;

      header->init(console, config);

      stream.init(header->version);
      text.init(header->version, header->abbr);
      parser.init(header->version);
      object.init(header->obj, header->version);

      ZState::init(options.seed,
                   sizeof(ZHeader),
                   header->getStorySize(),
                   header->glob,
                   header->getMemoryLimit());

      initDecoder();

      info("Version : z%d\n", header->version);

      start(restore_save);

      if(options.trace)
      {
         while(!isQuitRequested())
         {
            printTrace();
            fetchDecodeExecute();
         }
      }
      else
      {
         while(!isQuitRequested())
         {
            fetchDecodeExecute();
         }
      }

      console.waitForKey();

      info("quit");

      Error exit_code = ZState::getExitCode();
      if(isError(exit_code))
      {
         char text[128];
         disassemble(text);
         error("PC=%s : %s", text, errorString(exit_code));
         return false;
      }

      return true;
   }
};


#endif
