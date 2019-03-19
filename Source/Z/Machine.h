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

#ifndef Z_MACHINE_H
#define Z_MACHINE_H

#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "share/Machine.h"

#include "Z/Config.h"
#include "Z/Disassembler.h"
#include "Z/Header.h"
#include "Z/Object.h"
#include "Z/Parser.h"
#include "Z/State.h"
#include "Z/Stream.h"
#include "Z/Text.h"
#include "Z/Screen.h"
#include "Z/Story.h"

namespace Z {

//! Z machine implementation
class Machine : public IF::Machine
{
public:
   Machine(Console& console_, const Options& options_, const Story& story_)
      : IF::Machine(console_, options_)
      , story_is_valid(story_.isValid())
      , state(story_, (const char*)options.save_dir, options.undo, options.seed)
      , dis(story_.getVersion())
      , stream(console, options_, story_.getVersion(), state.memory)
      , screen(console, stream, story_.getVersion())
      , object(state.memory)
      , text(story_.getHeader(), state.memory)
      , parser(story_.getVersion())
   {
      header = (Header*)state.memory.data();

      object.init(header->obj, header->version);

      initDecoder(story_.getVersion());
   }

   //! Play a Z file.
   //! \return true if there were no errors
   bool play(bool restore)
   {
      std::string text;

      text = "Version  : z";
      text += std::to_string(header->version);
      stream.info(text);

      text = "Checksum : ";
      // The proper C++ way of doing this is not appealing
      const char* hex_digs = "0123456789ABCDEF";
      uint16_t checksum = header->checksum;
      text += hex_digs[(checksum >>  4) & 0xF];
      text += hex_digs[(checksum >>  0) & 0xF];
      text += hex_digs[(checksum >> 12) & 0xF];
      text += hex_digs[(checksum >>  8) & 0xF];
      stream.info(text);

      if((header->version >= 3) && !story_is_valid)
      {
         if (header->version == 3)
         {
            // Some v3 games do not have a checksum
            stream.info("Checksum fail");
         }
         else
         {
            stream.warning("Checksum fail");
         }
      }

      reset(restore);

      bool ok = true;

      try
      {
         if(options.trace)
         {
            while(!state.isQuitRequested())
            {
               inst_addr = state.getPC();
               dis.trace(dis_text, inst_addr, state.memory.data() + inst_addr);
               stream.getTrace().write(dis_text);
               fetchDecodeExecute();
            }
         }
         else
         {
            while(!state.isQuitRequested())
            {
               inst_addr = state.getPC();
               fetchDecodeExecute();
            }
         }

         if (header->version <= 3) showStatus();
      }
      catch(const char* message)
      {
         if (strcmp(message, "quit") != 0)
         {
            (void) dis.disassemble(dis_text, inst_addr, state.memory.data() + inst_addr);
            dis_text += " \"";
            dis_text += message;
            dis_text += "\"";
            stream.error(dis_text);
            ok = false;
         }
      }

      console.waitForKey();

      stream.info("quit");

      return ok;
   }

private:
   typedef void (Machine::*OpPtr)();

   static const unsigned MAX_OPERANDS = 8;

   Config       config;
   bool         story_is_valid;
   State        state;
   Disassembler dis;
   Stream       stream;
   Screen       screen;
   Object       object;
   Text         text;
   Parser       parser;
   Header*      header{};

   unsigned     num_arg;
   union
   {
      uint16_t uarg[MAX_OPERANDS];
      int16_t  sarg[MAX_OPERANDS];
   };

   // Op-code decoders
   OpPtr op0[0x10];
   OpPtr op1[0x10];
   OpPtr op2[0x20];
   OpPtr opV[0x20];
   OpPtr opE[0x20];

   // string used in multiple places, (possibly overly cautious
   // of dynamic memory allocation) but use of this string keeps
   // allocations to a minimum
   std::string work_str;

   //! Check for v3 time games
   bool isTimeGame() const
   {
      return (header->version == 3) &&
             ((header->flags1 & (1<<1)) != 0);
   }

   //! Conditional branch (4.7)
   void branch(bool cond)
   {
      uint8_t type           = state.fetch8();
      bool    branch_if_true = (type & (1 << 7)) != 0;
      bool    long_branch    = (type & (1 << 6)) == 0;
      int16_t offset         = type & 0x3F;

      if(long_branch)
      {
         offset = (offset << 8) | state.fetch8();
         // Sign extend
         offset = int16_t(offset << 2) >> 2;
      }

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
            state.branch(offset - 2);
         }
      }
   }

   //! Call a sub-routine
   void subCall(uint16_t        call_type,
                uint16_t        packed_addr,
                uint16_t        argc = 0,
                const uint16_t* argv = nullptr)
   {
      uint32_t target = header->unpackAddr(packed_addr, /* routine */ true);
      if (target == 0)
      {
         // this is legal, just return false
         switch(call_type)
         {
         case 0:  state.varWrite(state.fetch8(), 0); break;
         case 1:  /* throw return value away */ break;
         case 2:  state.push(0); break;
         case 3:  state.varWrite(state.fetch8(), 0); break;
         default: throw "bad call type"; break;
         }
         return;
      }

      if (call_type == 3)
      {
         state.push(packed_addr);
      }

      state.call(call_type, target);

      uint8_t num_locals = state.fetch8();

      state.push(argc);

      for(unsigned i = 0; i < num_locals; ++i)
      {
         uint16_t value = 0;

         if(header->version <= 4)
         {
            value = state.fetch16();
         }

         if(i < argc)
         {
            value = argv[i];
         }

         state.push(value);
      }
   }

   //! Return from a sub-routine
   void subRet(uint16_t value, uint16_t frame_ptr = 0xFFFF)
   {
      if(frame_ptr == 0xFFFF)
      {
         frame_ptr = state.getFramePtr();
      }

      uint8_t  call_type = state.returnFromFrame(frame_ptr);

      switch(call_type)
      {
      case 0: state.varWrite(state.fetch8(), value); break;
      case 1: /* throw return value away */ break;
      case 2: state.push(value);            break;

      case 3:
         {
            uint16_t packed_routine = state.pop();
            uint16_t timeout        = state.pop();
            if (value == 0)
            {
               uint16_t zscii;
               if (readChar(timeout, /* echo */ false, packed_routine, zscii))
               {
                  state.varWrite(state.fetch8(), zscii);
               }
            }
            else
            {
               state.varWrite(state.fetch8(), value);
            }
         }
         break;

      default: throw "bad call type";
      }
   }

   bool readChar(uint16_t timeout, bool echo, uint16_t routine, uint16_t& zscii)
   {
      if(!stream.readChar(zscii, timeout, echo))
      {
         // Timeout
         if(routine != 0)
         {
            state.push(timeout);
            subCall(/* call_type */ 3, routine);
         }
         return false;
      }

      // Character available

      // Newline is 13 in ZSCII
      if (zscii == '\n') zscii = 13;

      return true;
   }

   void showStatus()
   {
      unsigned num_cols = screen.getWidth();
      unsigned limit    = isTimeGame() ? num_cols - 61
                                       : num_cols - 27;

      work_str = " ";

      uint16_t loc  = state.varRead(16+0);
      uint32_t name = object.getName(loc);
      text.print([this, limit](uint16_t ch)
                 {
                    if (work_str.size() < limit)
                    {
                       work_str += ch;
                    }
                 },
                 name);

      while(work_str.size() < limit)
      {
         work_str += " ";
      }

      if (isTimeGame())
      {
         uint16_t hours = state.varRead(16+1);
         uint16_t mins  = state.varRead(16+2);

         work_str += "Time : ";
         if (hours<10) work_str += '0';
         work_str += std::to_string(hours);
         work_str += ':';
         if (mins<10) work_str += '0';
         work_str += std::to_string(mins);
      }
      else
      {
         uint16_t moves = state.varRead(16+2);
         work_str += "Moves: ";
         work_str += std::to_string(moves);

         while(work_str.size() < (num_cols - 14))
         {
            work_str += " ";
         }

         int16_t  score = state.varRead(16+1);
         work_str += "Score : ";
         work_str += std::to_string(score);
      }

      while(work_str.size() < num_cols)
      {
         work_str += " ";
      }

      screen.showStatus(work_str);
   }

   uint32_t streamText(uint32_t addr)
   {
      return text.print([this](uint16_t ch){ stream.writeChar(ch); }, addr);
   }

   //! Read filename from memory
   void readFilename(uint16_t name, std::string& filename)
   {
      // Make sure the save directory exists
      (void) PLT::File::createDir((const char*)options.save_dir);

      filename = (const char*)options.save_dir;
      filename += '/';
      uint8_t size = state.memory.read8(name);
      for(uint8_t i=0; i<size; i++)
      {
         filename += toupper(state.memory.read8(name+i+1));
      }
      if (filename.find('.') == std::string::npos)
      {
         filename += ".AUX";
      }
   }

   void ILLEGAL() { throw "illegal op"; }

   void TODO_WARN(const char* op) { stream.warning(op); }

   //============================================================================
   // Zero operand instructions

   //! rtrue - Return true (i.e. 1) from the current routine
   void op0_rtrue() { subRet(1); }

   //! rfalse - Return false (i.e. 0) from the current routine
   void op0_rfalse() { subRet(0); }

   //! print - Print the literal Z-encoded string
   void op0_print() { state.jump(streamText(state.getPC())); }

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
   void op0_save_v1() { branch(state.save()); }

   //! v4 save -> (result)
   void op0_save_v4()
   {
      uint8_t ret = state.fetch8();
      state.varWrite(ret, 2);
      state.varWrite(ret, state.save() ? 1 : 0);
   }

   //! v1 restore ?(label)
   void op0_restore_v1() { branch(reset(/* resotore */ true)); }

   //! v4 restore -> (result)
   void op0_restore_v4()
   {
      if(!reset(/* restore */ true)) state.varWrite(state.fetch8(), 0);
   }

   //! restart
   void op0_restart() { reset(false); }

   //! ret_popped
   void op0_ret_popped() { subRet(state.pop()); }

   //! pop
   void op0_pop() { state.pop(); }

   //! catch -> (result)
   void op0_catch() { state.varWrite(state.fetch8(), state.getFramePtr()); }

   //! quit
   void op0_quit() { state.quit(); }

   //! new_line
   void op0_new_line() { stream.writeChar('\n'); }

   //! show_status
   void op0_show_status() { showStatus(); }

   //! verify ?(label)
   void op0_verify() { branch(story_is_valid); }

   //! piracy ?(label)
   void op0_piracy() { branch(true); }

   //============================================================================
   // One operand instructions

   void op1_jz() { branch(uarg[0] == 0); }

   void op1_get_sibling()
   {
      uint16_t obj = object.getSibling(uarg[0]);
      state.varWrite(state.fetch8(), obj);
      branch(obj != 0);
   }

   void op1_get_parent()
   {
      uint16_t obj = object.getParent(uarg[0]);
      state.varWrite(state.fetch8(), obj);
   }

   void op1_get_child()
   {
      uint16_t obj = object.getChild(uarg[0]);
      state.varWrite(state.fetch8(), obj);
      branch(obj != 0);
   }

   void op1_get_prop_len()  { state.varWrite(state.fetch8(), object.propSize(uarg[0])); }

   void op1_inc()           { state.varWrite(uarg[0], state.varRead(uarg[0]) + 1); }

   void op1_dec()           { state.varWrite(uarg[0], state.varRead(uarg[0]) - 1); }

   void op1_print_addr()    { streamText(uarg[0]); }

   void op1_call_1s()       { subCall(0, uarg[0]); }

   void op1_remove_obj()    { object.remove(uarg[0]); }

   void op1_print_obj()     { streamText(object.getName(uarg[0])); }

   void op1_ret()           { subRet(uarg[0]); }

   void op1_jump()          { state.branch(sarg[0] - 2); }

   void op1_print_paddr()   { streamText(header->unpackAddr(uarg[0], /* routine */false)); }

   void op1_load()          { state.varWrite(state.fetch8(), state.varRead(uarg[0], true)); }

   void op1_not()           { state.varWrite(uarg[0], ~uarg[0]); }

   void op1_call_1n()       { subCall(1, uarg[0]); }

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
      int16_t value = state.varRead(uarg[0]) - 1;
      state.varWrite(uarg[0], value);
      branch(value < sarg[1]);
   }

   void op2_inc_chk()
   {
      int16_t value = state.varRead(uarg[0]) + 1;
      state.varWrite(uarg[0], value);
      branch(value > sarg[1]);
   }

   void op2_jin()           { branch(object.getParent(uarg[0]) == uarg[1]); }
   void op2_test_bitmap()   { branch((uarg[0] & uarg[1]) == uarg[1]); }
   void op2_or()            { state.varWrite(state.fetch8(), uarg[0] | uarg[1]); }
   void op2_and()           { state.varWrite(state.fetch8(), uarg[0] & uarg[1]); }
   void op2_test_attr()     { branch(object.getAttr(uarg[0], uarg[1])); }
   void op2_set_attr()      { object.setAttr(uarg[0], uarg[1], true); }
   void op2_clear_attr()    { object.setAttr(uarg[0], uarg[1], false); }
   void op2_store()         { state.varWrite(uarg[0], uarg[1], true); }
   void op2_insert_obj()    { object.insert(uarg[0], uarg[1]); }

   //! 2OP:15 0F loadw array word_index -> (result)
   void op2_loadw()
   {
      state.varWrite(state.fetch8(), state.memory.read16(uarg[0]+2*uarg[1]));
   }

   //! 2OP:16 10 loadb array byte_index -> (result)
   //  Stores array->byte_index (i.e., the byte at address array+byte_index,
   //  which must lie in static or dynamic memory)
   void op2_loadb()
   {
      state.varWrite(state.fetch8(), state.memory.read8(uarg[0] + uarg[1]));
   }

   void op2_get_prop()      { state.varWrite(state.fetch8(), object.getProp(uarg[0], uarg[1])); }
   void op2_get_prop_addr() { state.varWrite(state.fetch8(), object.getPropAddr(uarg[0], uarg[1])); }
   void op2_get_next_prop() { state.varWrite(state.fetch8(), object.getPropNext(uarg[0], uarg[1])); }
   void op2_add()           { state.varWrite(state.fetch8(), sarg[0] + sarg[1]); }
   void op2_sub()           { state.varWrite(state.fetch8(), sarg[0] - sarg[1]); }
   void op2_mul()           { state.varWrite(state.fetch8(), sarg[0] * sarg[1]); }

   void op2_div()
   {
      if(sarg[1] == 0)
      {
         throw "div by zero";
         return;
      }
      state.varWrite(state.fetch8(), sarg[0] / sarg[1]);
   }

   void op2_mod()
   {
      if(sarg[1] == 0)
      {
         throw "div by zero";
         return;
      }
      state.varWrite(state.fetch8(), sarg[0] % sarg[1]);
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
         state.varWrite(state.fetch8(), 0);
      else
         subCall(0, uarg[0], num_arg-1, &uarg[1]);
   }

   void opV_call_vs()        { opV_call(); }
   void opV_not()            { state.varWrite(state.fetch8(), ~uarg[0]); }
   void opV_call_vn()        { subCall(1, uarg[0], num_arg-1, &uarg[1]); }
   void opV_call_vn2()       { opV_call_vn(); }
   void opV_storew()         { state.memory.write16(uarg[0] + 2*uarg[1], uarg[2]); }
   void opV_storeb()         { state.memory.write8(uarg[0] + uarg[1], uarg[2]); }
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

      uint8_t  len   = 0;
      uint8_t  max   = state.memory.read8(buffer++) - 1;
      uint16_t start = buffer;

      while(len < max)
      {
         uint16_t zscii;

         if(!readChar(timeout, /* echo */ true, routine, zscii))
         {
            return;
         }

         if(zscii == '\b')
         {
            // => delete
            if(buffer > start)
            {
               stream.deleteChar();
               --buffer;
               --len;
            }
         }
         else if(zscii == 13)
         {
            state.memory.write8(buffer, '\0');
            break;
         }
         else
         {
            state.memory.write8(buffer++, tolower(zscii));
            len++;
         }
      }

      parser.tokenise(state.memory, parse, start, header->dict, false);
   }

   //! aread text parse timeout routine -> (result)
   void opV_aread()
   {
      uint16_t buffer  = uarg[0];
      uint16_t parse   = uarg[1];
      uint16_t timeout = num_arg >= 3 ? uarg[2] : 0;
      uint16_t routine = num_arg >= 4 ? uarg[3] : 0;

      uint8_t max = state.memory.read8(buffer++);
      uint8_t len = state.memory.read8(buffer++);

      uint16_t start  = buffer;
      uint8_t  status = 0;

      buffer += len;

      while(len < max)
      {
         uint16_t zscii;

         if(!readChar(timeout, /* echo */ true, routine, zscii))
         {
            break;
         }

         if(zscii == '\b')
         {
            // => delete
            if(buffer > start)
            {
               stream.deleteChar();
               --buffer;
               --len;
            }
         }
         else if(zscii == 13)
         {
            state.memory.write8(buffer, '\0');
            state.memory.write8(start - 1, len);
            status = uint8_t(zscii);
            break;
         }
         else
         {
            state.memory.write8(buffer++, tolower(zscii));
            len++;
         }
      }

      state.varWrite(state.fetch8(), status);

      if(parse != 0)
      {
         parser.tokenise(state.memory, parse, start, header->dict, false);
      }
   }

   void opV_print_char()     { stream.writeChar(uarg[0]); }
   void opV_print_num()      { stream.writeNumber(sarg[0]); }
   void opV_random()         { state.varWrite(state.fetch8(), state.randomOp(sarg[0])); }
   void opV_push()           { state.push(uarg[0]); }

   void opV_pull_v1()
   {
      state.varWrite(uarg[0], state.pop(), true);
   }

   void opV_pull_v6()
   {
      uint16_t value;
      if(num_arg == 1)
      {
         // User stack
         uint16_t st  = uarg[0];
         uint16_t ptr = state.memory.read16(st);
         state.memory.write16(st, ++ptr);
         value = state.memory.read16(ptr + 2 * ptr);
      }
      else
      {
         value = state.pop();
      }

      state.varWrite(state.fetch8(), value, true);
   }

   void opV_split_window()   { screen.splitWindow(uarg[0]); }
   void opV_set_window()     { screen.selectWindow(uarg[0]); }
   void opV_call_vs2()       { subCall(0, uarg[0], num_arg - 1, &uarg[1]); }
   void opV_erase_window()   { screen.eraseWindow(sarg[0]); }

   void opV_erase_line_v4()
   {
      if (uarg[0] == 1)
         screen.eraseLine();
   }

   void opV_erase_line_v6()
   {
      if (uarg[0] == 1)
         screen.eraseLine();
      else
         TODO_WARN("v6 op erase_line pixels unimplemented");
   }

   void opV_set_cursor_v4()
   {
      screen.moveCursor(sarg[0], uarg[1]);
   }

   void opV_set_cursor_v6()
   {
      screen.moveCursor(sarg[0], uarg[1], uarg[2]);
   }

   void opV_get_cursor()
   {
      unsigned row, col;
      screen.getCursor(row, col);

      uint16_t array = uarg[0];
      state.memory.write16(array + 0, row);
      state.memory.write16(array + 2, col);
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
         int16_t stream_idx = abs(number);

         if(stream_idx > 4)
            throw "bad stream";
         else if(number > 0)
            stream.enableStream(stream_idx, true);
         else if(number < 0)
            stream.enableStream(stream_idx, false);
      }
   }

   void opV_input_stream() { throw "op input_stream unimplemented"; }

   void opV_sound_effect() { TODO_WARN("op sound_effect unimplemeneted"); }

   void opV_read_char()
   {
      // assert(uarg[0] == 1);
      uint16_t timeout = num_arg >= 2 ? uarg[1] : 0;
      uint16_t routine = num_arg >= 3 ? uarg[2] : 0;
      uint16_t zscii;

      if(readChar(timeout, /* echo */ false, routine, zscii))
      {
         state.varWrite(state.fetch8(), zscii);
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
         uint16_t v = (form & 0x80) ? state.memory.read16(table)
                                    : state.memory.read8(table);

         if(v == x)
         {
            result = table;
            break;
         }

         table += form & 0x7F;
      }

      state.varWrite(state.fetch8(), result);

      branch(result != 0);
   }

   void opV_tokenise()
   {
      uint16_t text  = uarg[0];
      uint16_t parse = uarg[1];
      uint16_t dict  = num_arg >= 3 ? uarg[2] : uint16_t(header->dict);
      bool     flag  = num_arg == 4 ? uarg[3] != 0 : false;

      // Test +2 to skip max len and actual len
      parser.tokenise(state.memory, parse, text + 2, dict, flag);
   }

   void opV_encode_text() { throw "op encode_text unimplemeneted"; }

   void opV_copy_table()
   {
      uint16_t from = uarg[0];
      uint16_t to   = uarg[1];
      int16_t  size = sarg[2];

      if(to == 0)
      {
         for(int16_t i = 0; i<size; i++)
         {
            state.memory.write8(from + i, 0);
         }
      }
      else if((size < 0) || (from > to))
      {
         for(int16_t i = 0; i < abs(size); i++)
         {
            state.memory.write8(to + i, state.memory.read8(from + i));
         }
      }
      else
      {
         for(int16_t i = size - 1; i >= 0; i--)
         {
            state.memory.write8(to + i, state.memory.read8(from + i));
         }
      }
   }

   void opV_print_table()
   {
      uint16_t addr   = uarg[0];
      uint16_t width  = uarg[1];
      uint16_t height = num_arg >= 3 ? uarg[2] : 1;
      uint16_t skip   = num_arg == 4 ? uarg[3] : 0;

      unsigned line, col;
      console.getCursorPos(line, col);

      for(unsigned l = 0; l < height; l++)
      {
         for(unsigned c = 0; c < width; c++)
         {
            uint8_t ch = state.memory.fetch8(addr++);
            stream.writeChar(ch);
         }

         console.moveCursor(line + l + 1, col);

         addr += skip;
      }
   }

   void opV_check_arg_count() { branch(uarg[0] <= state.getNumFrameArgs()); }

   //============================================================================
   // Extended operand instructions

   void opE_save_table()
   {
      bool    ok  = false;
      uint8_t ret = state.fetch8();

      if (num_arg == 3)
      {
         uint16_t table = uarg[0];
         uint16_t bytes = uarg[1];
         uint16_t name  = uarg[2];

         std::string filename;
         readFilename(name, filename);
         FILE* fp = fopen(filename.c_str(), "w");
         if (fp != nullptr)
         {
            for(unsigned i=0; i<bytes; i++)
            {
               fputc(state.memory.read8(table + i), fp);
            }
            fclose(fp);
            ok = true;
         }
      }
      else
      {
         state.varWrite(ret, 2);
         ok = state.save();
      }

      state.varWrite(ret, ok ? 1 : 0);
   }

   void opE_restore_table()
   {
      if (num_arg == 3)
      {
         uint16_t table = uarg[0];
         uint16_t size  = uarg[1];
         uint16_t name  = uarg[2];

         uint16_t bytes = 0;

         std::string filename;
         readFilename(name, filename);
         FILE* fp = fopen(filename.c_str(), "r");
         if (fp != nullptr)
         {
            for(bytes=0; bytes<size; bytes++)
            {
               int ch = fgetc(fp);
               if (ch < 0)
               {
                  break;
               }
               state.memory.write8(table + bytes, uint8_t(ch));
            }
            fclose(fp);
         }

         state.varWrite(state.fetch8(), bytes);
      }
      else if(!reset(/* restore */ true))
      {
         state.varWrite(state.fetch8(), 0);
      }
   }

   void opE_log_shift()
   {
      if(sarg[1] < 0)
         state.varWrite(state.fetch8(), uarg[0] >> -sarg[1]);
      else
         state.varWrite(state.fetch8(), uarg[0] << sarg[1]);
   }

   void opE_art_shift()
   {
      if(sarg[1] < 0)
         state.varWrite(state.fetch8(), sarg[0] >> -sarg[1]);
      else
         state.varWrite(state.fetch8(), sarg[0] << sarg[1]);
   }

   void opE_save_undo()
   {
      uint8_t ret = state.fetch8();
      state.varWrite(ret, 2);
      state.varWrite(ret, state.saveUndo() ? 1 : 0);
   }

   void opE_restore_undo()
   {
      if(!state.restoreUndo())
      {
         state.varWrite(state.fetch8(), 0);
      }
   }

   void opE_print_unicode()
   {
      uint16_t code = uarg[0];

      if ((code >= 0x20) && (code <= 0x7E))
      {
         stream.writeChar(code);
      }
      else
      {
         switch(code)
         {
         case 0x00A9: // Copyright
            stream.writeChar('(');
            stream.writeChar('C');
            stream.writeChar(')');
            break;

         case 0x0219: // Latin small s with comma below
            stream.writeChar('s');
            break;

         case 0x2014: // Em dash
            stream.writeChar('-');
            break;

         case 0x2026: // Horizontal ellipses
            stream.writeChar('.');
            stream.writeChar('.');
            stream.writeChar('.');
            break;

         case 0x2212: // Minus sign
            stream.writeChar('-');
            break;

         default:
            work_str = "unsupported unicode ";
            work_str += std::to_string(code);
            TODO_WARN(work_str.c_str());
            code = '?';
            break;
         }
      }
   }

   void opE_check_unicode()
   {
      uint16_t code = uarg[0];
      uint16_t bit_mask = 0;

      if ((code >= 0x20) && (code <= 0x7E))
      {
         bit_mask = 0b11;
      }
      else
      {
         switch(code)
         {
         case 0x00A9: // Copyright
         case 0x0219: // Latin small s with comma below
         case 0x2014: // Em Dash
         case 0x2026: // Horizontal ellipses
         case 0x2212: // Minus sign
            bit_mask = 0b01;
            break;
         }
      }

      state.varWrite(state.fetch8(), bit_mask);
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

      state.memory.write16(array + 0, value1);
      state.memory.write16(array + 2, value2);

      branch(valid);
   }

   void opE_erase_picture() { TODO_WARN("op erase_picture unimplemented"); }

   void opE_set_margins() { TODO_WARN("op set_margins unimplemented"); }

   //! EXT:4
   //  set_font font
   void opE_set_font()
   {
      bool ok = stream.setFont(uarg[0]);
      state.varWrite(state.fetch8(), ok);
   }

   void opE_move_window()
   {
      uint16_t wind = uarg[0];
      uint16_t y    = uarg[1];
      uint16_t x    = uarg[2];

      screen.moveWindow(wind, y, x);
   }

   void opE_window_size()
   {
      uint16_t wind = uarg[0];
      uint16_t y    = uarg[1];
      uint16_t x    = uarg[2];

      screen.resizeWindow(wind, y, x);
   }

   void opE_window_style()
   {
      uint16_t wind      = uarg[0];
      uint16_t flags     = uarg[1];
      uint16_t operation = uarg[2];

      screen.setWindowStyle(wind, flags, operation);
   }

   void opE_get_wind_prop()
   {
      uint16_t wind = uarg[0];
      uint16_t prop = uarg[1];

      state.varWrite(state.fetch8(), screen.getWindowProp(wind, prop));
   }

   void opE_scroll_window()
   {
      uint16_t wind   = uarg[0];
      uint16_t pixels = uarg[1];

      screen.scrollWindow(wind, pixels);
   }

   void opE_pop_stack()
   {
      uint16_t items = uarg[0];

      if (num_arg == 1)
      {
         uint16_t stack = uarg[1];
         uint16_t size  = state.memory.read16(stack);

         size += items;

         state.memory.write16(stack, size);
      }
      else
      {
         for(uint16_t i=0; i<items; i++)
         {
            (void) state.pop();
         }
      }
   }

   void opE_read_mouse() { throw "read_mouse unimplemented"; }

   void opE_mouse_window() { TODO_WARN("mouse_window unimplemented"); }

   void opE_push_stack()
   {
      uint16_t value = uarg[0];
      uint16_t stack = uarg[1];
      uint16_t size  = state.memory.read16(stack);

      if (size != 0)
      {
         state.memory.write16(stack + 2*size, value);
         --size;
         state.memory.write16(stack, size);
      }

      branch(size != 0);
   }

   void opE_put_wind_prop()
   {
      uint16_t wind  = uarg[0];
      uint16_t prop  = uarg[1];
      uint16_t value = uarg[2];

      screen.setWindowProp(wind, prop, value);
   }

   void opE_print_form()
   {
      uint16_t formatted_table = uarg[0];

      text.printForm([this](uint16_t ch){ stream.writeChar(ch); }, formatted_table);
   }

   void opE_make_menu()
   {
      uint16_t number = uarg[0];
      uint16_t table  = uarg[1];

      branch(false);

      (void) number;
      (void) table;

      throw "make_menu unimplemented";
   }

   void opE_picture_table() { TODO_WARN("picture_table unimplemented"); }

   void initDecoder(unsigned version)
   {
      // Zero operand instructions
      op0[0x0] =                &Machine::op0_rtrue;
      op0[0x1] =                &Machine::op0_rfalse;
      op0[0x2] =                &Machine::op0_print;
      op0[0x3] =                &Machine::op0_print_ret;
      op0[0x4] =                &Machine::op0_nop;
      op0[0x5] = version <= 3 ? &Machine::op0_save_v1
               : version == 4 ? &Machine::op0_save_v4
                              : &Machine::ILLEGAL;
      op0[0x6] = version <= 3 ? &Machine::op0_restore_v1
               : version == 4 ? &Machine::op0_restore_v4
                              : &Machine::ILLEGAL;
      op0[0x7] =                &Machine::op0_restart;
      op0[0x8] =                &Machine::op0_ret_popped;
      op0[0x9] = version <= 4 ? &Machine::op0_pop
                              : &Machine::op0_catch;
      op0[0xA] =                &Machine::op0_quit;
      op0[0xB] =                &Machine::op0_new_line;
      op0[0xC] = version <= 2 ? &Machine::ILLEGAL
               : version == 3 ? &Machine::op0_show_status
                              : &Machine::op0_nop;
      op0[0xD] = version >= 3 ? &Machine::op0_verify
                              : &Machine::ILLEGAL;
      op0[0xE] =                &Machine::ILLEGAL;   // "extend" decoded elsewhere
      op0[0xF] = version >= 5 ? &Machine::op0_piracy
                              : &Machine::ILLEGAL;

      // One operand instructions
      op1[0x0] =                &Machine::op1_jz;
      op1[0x1] =                &Machine::op1_get_sibling;
      op1[0x2] =                &Machine::op1_get_child;
      op1[0x3] =                &Machine::op1_get_parent;
      op1[0x4] =                &Machine::op1_get_prop_len;
      op1[0x5] =                &Machine::op1_inc;
      op1[0x6] =                &Machine::op1_dec;
      op1[0x7] =                &Machine::op1_print_addr;
      op1[0x8] = version >= 4 ? &Machine::op1_call_1s
                              : &Machine::ILLEGAL;
      op1[0x9] =                &Machine::op1_remove_obj;
      op1[0xA] =                &Machine::op1_print_obj;
      op1[0xB] =                &Machine::op1_ret;
      op1[0xC] =                &Machine::op1_jump;
      op1[0xD] =                &Machine::op1_print_paddr;
      op1[0xE] =                &Machine::op1_load;
      op1[0xF] = version <= 4 ? &Machine::op1_not
                              : &Machine::op1_call_1n;

      // Two operand instructions
      op2[0x00] =                &Machine::ILLEGAL;
      op2[0x01] =                &Machine::op2_je;
      op2[0x02] =                &Machine::op2_jl;
      op2[0x03] =                &Machine::op2_jg;
      op2[0x04] =                &Machine::op2_dec_chk;
      op2[0x05] =                &Machine::op2_inc_chk;
      op2[0x06] =                &Machine::op2_jin;
      op2[0x07] =                &Machine::op2_test_bitmap;
      op2[0x08] =                &Machine::op2_or;
      op2[0x09] =                &Machine::op2_and;
      op2[0x0A] =                &Machine::op2_test_attr;
      op2[0x0B] =                &Machine::op2_set_attr;
      op2[0x0C] =                &Machine::op2_clear_attr;
      op2[0x0D] =                &Machine::op2_store;
      op2[0x0E] =                &Machine::op2_insert_obj;
      op2[0x0F] =                &Machine::op2_loadw;
      op2[0x10] =                &Machine::op2_loadb;
      op2[0x11] =                &Machine::op2_get_prop;
      op2[0x12] =                &Machine::op2_get_prop_addr;
      op2[0x13] =                &Machine::op2_get_next_prop;
      op2[0x14] =                &Machine::op2_add;
      op2[0x15] =                &Machine::op2_sub;
      op2[0x16] =                &Machine::op2_mul;
      op2[0x17] =                &Machine::op2_div;
      op2[0x18] =                &Machine::op2_mod;
      op2[0x19] = version >= 4 ? &Machine::op2_call_2s
                               : &Machine::ILLEGAL;
      op2[0x1A] = version >= 5 ? &Machine::op2_call_2n
                               : &Machine::ILLEGAL;
      op2[0x1B] = version >= 5 ? &Machine::op2_set_colour
                               : &Machine::ILLEGAL;
      op2[0x1C] = version >= 5 ? &Machine::op2_throw
                               : &Machine::ILLEGAL;
      op2[0x1D] =                &Machine::ILLEGAL;
      op2[0x1E] =                &Machine::ILLEGAL;
      op2[0x1F] =                &Machine::ILLEGAL;

      // Variable operand instructions
      opV[0x00] = version <= 3 ? &Machine::opV_call
                               : &Machine::opV_call_vs;
      opV[0x01] =                &Machine::opV_storew;
      opV[0x02] =                &Machine::opV_storeb;
      opV[0x03] =                &Machine::opV_put_prop;
      opV[0x04] = version <= 3 ? &Machine::opV_sread<false,true>
                : version == 4 ? &Machine::opV_sread<true,false>
                               : &Machine::opV_aread;
      opV[0x05] =                &Machine::opV_print_char;
      opV[0x06] =                &Machine::opV_print_num;
      opV[0x07] =                &Machine::opV_random;
      opV[0x08] =                &Machine::opV_push;
      opV[0x09] = version == 6 ? &Machine::opV_pull_v6
                               : &Machine::opV_pull_v1;
      opV[0x0A] = version >= 3 ? &Machine::opV_split_window
                               : &Machine::ILLEGAL;
      opV[0x0B] = version >= 3 ? &Machine::opV_set_window
                               : &Machine::ILLEGAL;
      opV[0x0C] = version >= 4 ? &Machine::opV_call_vs2
                               : &Machine::ILLEGAL;
      opV[0x0D] = version >= 4 ? &Machine::opV_erase_window
                               : &Machine::ILLEGAL;
      opV[0x0E] = version >= 4 ? &Machine::opV_erase_line_v4
                : version >= 6 ? &Machine::opV_erase_line_v6
                               : &Machine::ILLEGAL;
      opV[0x0F] = version >= 4 ? &Machine::opV_set_cursor_v4
                : version >= 6 ? &Machine::opV_set_cursor_v6
                               : &Machine::ILLEGAL;
      opV[0x10] = version >= 4 ? &Machine::opV_get_cursor      : &Machine::ILLEGAL;
      opV[0x11] = version >= 4 ? &Machine::opV_set_text_style  : &Machine::ILLEGAL;
      opV[0x12] = version >= 4 ? &Machine::opV_buffer_mode     : &Machine::ILLEGAL;
      opV[0x13] = version >= 3 ? &Machine::opV_output_stream   : &Machine::ILLEGAL;
      opV[0x14] = version >= 3 ? &Machine::opV_input_stream    : &Machine::ILLEGAL;
      opV[0x15] = version >= 5 ? &Machine::opV_sound_effect    : &Machine::ILLEGAL;
      opV[0x16] = version >= 4 ? &Machine::opV_read_char       : &Machine::ILLEGAL;
      opV[0x17] = version >= 4 ? &Machine::opV_scan_table      : &Machine::ILLEGAL;
      opV[0x18] = version >= 5 ? &Machine::opV_not             : &Machine::ILLEGAL;
      opV[0x19] = version >= 5 ? &Machine::opV_call_vn         : &Machine::ILLEGAL;
      opV[0x1A] = version >= 5 ? &Machine::opV_call_vn2        : &Machine::ILLEGAL;
      opV[0x1B] = version >= 5 ? &Machine::opV_tokenise        : &Machine::ILLEGAL;
      opV[0x1C] = version >= 5 ? &Machine::opV_encode_text     : &Machine::ILLEGAL;
      opV[0x1D] = version >= 5 ? &Machine::opV_copy_table      : &Machine::ILLEGAL;
      opV[0x1E] = version >= 5 ? &Machine::opV_print_table     : &Machine::ILLEGAL;
      opV[0x1F] = version >= 5 ? &Machine::opV_check_arg_count : &Machine::ILLEGAL;

      // Externded instructions
      for(unsigned i = 0; i <= 0x1F; i++)
      {
         opE[i] = &Machine::ILLEGAL;
      }

      if(version < 5) return;

      opE[0x00] = &Machine::opE_save_table;
      opE[0x01] = &Machine::opE_restore_table;
      opE[0x02] = &Machine::opE_log_shift;
      opE[0x03] = &Machine::opE_art_shift;
      opE[0x04] = &Machine::opE_set_font;
      opE[0x09] = &Machine::opE_save_undo;
      opE[0x0A] = &Machine::opE_restore_undo;
      opE[0x0B] = &Machine::opE_print_unicode;
      opE[0x0C] = &Machine::opE_check_unicode;

      if(version != 6) return;

      opE[0x05] = &Machine::opE_draw_picture;
      opE[0x06] = &Machine::opE_picture_data;
      opE[0x07] = &Machine::opE_erase_picture;
      opE[0x08] = &Machine::opE_set_margins;

      opE[0x10] = &Machine::opE_move_window;
      opE[0x11] = &Machine::opE_window_size;
      opE[0x12] = &Machine::opE_window_style;
      opE[0x13] = &Machine::opE_get_wind_prop;
      opE[0x14] = &Machine::opE_scroll_window;
      opE[0x15] = &Machine::opE_pop_stack;
      opE[0x16] = &Machine::opE_read_mouse;
      opE[0x17] = &Machine::opE_mouse_window;
      opE[0x18] = &Machine::opE_push_stack;
      opE[0x19] = &Machine::opE_put_wind_prop;
      opE[0x1A] = &Machine::opE_print_form;
      opE[0x1B] = &Machine::opE_make_menu;
      opE[0x1C] = &Machine::opE_picture_table;
   }

   //============================================================================

   void clearOperands() { num_arg = 0; }

   void fetchOperand(OperandType type)
   {
      uint16_t operand;

      switch(type)
      {
      case OP_LARGE_CONST: operand = state.fetch16();               break;
      case OP_SMALL_CONST: operand = state.fetch8();                break;
      case OP_VARIABLE:    operand = state.varRead(state.fetch8()); break;
      default: assert(!"bad operand type"); return;
      }

      uarg[num_arg++] = operand;

      assert(num_arg <= 8);
   }

   //! Fetch variable number of operands
   void fetchOperands(unsigned max_num_operands)
   {
      uint16_t op_types;

      if(max_num_operands == 4)
      {
         op_types = state.fetch8() << 8;
      }
      else
      {
         assert(max_num_operands == 8);

         op_types = state.fetch16();
      }

      // Unpack the type of the operands
      for(unsigned i = 0; i < max_num_operands; ++i)
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

   //! Reset the interpreter to initial conditions
   bool reset(bool restore)
   {
      bool ok = true;

      if (restore)
      {
         ok = state.restore();
      }
      else
      {
         state.reset();
      }

      if (ok)
      {
         screen.reset();
         header->reset(console, config);
      }

      return ok;
   }

   void fetchDecodeExecute()
   {
      uint8_t opcode = state.fetch8();

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
            doOpE(state.fetch8());
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
};

} // namespace Z

#endif
