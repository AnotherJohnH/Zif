//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cmath>
#include <cstring>

#include "common/Log.h"
#include "common/Machine.h"

#include "Glulx/State.h"
#include "Glulx/Story.h"
#include "Glulx/Disassembler.h"

namespace Glulx {

//! Glulx machine implementation
class Machine : public IF::Machine
{
public:
   Machine(Console& console_, const Options& options_, const Glulx::Story& story_)
      : IF::Machine(console_, options_)
      , state(story_, (const char*)options.save_dir, options.undo, options.seed)
   {
   }

   bool play(bool restore)
   {
      bool ok = true;

      state.reset();

      try
      {
         // PC after reset points at the first function not code
         call(state.getPC(), 0);

         if (options.trace)
         {
            while(!state.isQuitRequested())
            {
               inst_addr = state.getPC();
               dis.trace(dis_text, inst_addr, state.memory.data() + inst_addr);
               trace.write(dis_text);
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
      }
      catch(const char* message)
      {
         if (strcmp(message, "quit") != 0)
         {
            (void) dis.disassemble(dis_text, inst_addr, state.memory.data() + inst_addr);
            dis_text += " \"";
            dis_text += message;
            dis_text += "\"";
            console.error(dis_text);
            ok = false;
         }
      }

      console.error("Glulx games are not currently supported");

      console.waitForKey();

      return ok;
   }

private:
   static const unsigned MAX_OPERAND = 4;

   State                state;
   IF::Memory::Address  ramstart{0};
   IF::Stack::Offset    local{0};
   Disassembler         dis;
   Log                  trace{"trace.log"};

   //! Decoded instruction address modes
   uint8_t  mode[MAX_OPERAND];
   uint32_t addr[MAX_OPERAND];

   //! Fetch opcode
   uint32_t fetchOp()
   {
      uint32_t op = state.fetch8();
      if (op >= 0xC0)
      {
         // 28-bit op-code 0..0x0FFFFFFF (does not appear to be used)
         op = ((op & 0x0F) << 24) | state.fetch24();
      }
      else if (op >= 0x80)
      {
         // 14-bit op-code 0..0x3FFFF (this is more than enough, methinks)
         op = ((op & 0x3F) << 8) | state.fetch8();
      }
      else
      {
         // 7-bit op-code 0..0x7F (should have been enough)
      }

      return op;
   }

   //! Fetch operand addresses
   void fetchA(unsigned n)
   {
      assert(n <= MAX_OPERAND);

      // Fetch and split address mode nibbles
      for(unsigned i=0; i<n; i += 2)
      {
         uint8_t mode_pair = state.fetch8();
         mode[i]   = mode_pair & 0xF;
         mode[i+1] = mode_pair >> 4;
      }

      // Pre-fetch operand addresses based on the LS 2 bits of the address mode
      for(unsigned i=0; i<n; i++)
      {
         switch(mode[i] & 0b11)
         {
         case 0: break;
         case 1: addr[i] = state.fetch8();  break;
         case 2: addr[i] = state.fetch16(); break;
         case 3: addr[i] = state.fetch32(); break;
         }
      }
   }

   //! Load an operand
   template <typename TYPE>
   TYPE loadOperand(unsigned i)
   {
      switch(mode[i])
      {
      case 0x0: return 0;
      case 0x1: return TYPE(int32_t(addr[i]<<24)>>24);
      case 0x2: return TYPE(int32_t(addr[i]<<16)>>16);
      case 0x3: return TYPE(addr[i]);
      case 0x4: throw "bad address mode";
      case 0x5:
      case 0x6:
      case 0x7: return state.memory.read<TYPE>(addr[i]);
      case 0x8: return TYPE(state.stack.pop32());
      case 0x9:
      case 0xA:
      case 0xB: return state.stack.read<TYPE>(local + addr[i]);
      case 0xC: throw "bad address mode";
      case 0xD:
      case 0xE:
      case 0xF: return state.memory.read<TYPE>(ramstart + addr[i]);

      default:
         assert(!"bad mode");
         return 0;
      }
   }

   //! Store a 32-bit integer operand 
   template <typename TYPE>
   void storeOperand(unsigned i, TYPE value)
   {
      switch(mode[i])
      {
      case 0x0: break;
      case 0x1:
      case 0x2:
      case 0x3:
      case 0x4: throw "bad address mode";
      case 0x5:
      case 0x6:
      case 0x7: state.memory.write<TYPE>(addr[i], value); break;
      case 0x8: state.stack.push32(value); break;
      case 0x9:
      case 0xA:
      case 0xB: state.stack.write<TYPE>(local + addr[i], value); break;
      case 0xC: throw "bad address mode";
      case 0xD:
      case 0xE:
      case 0xF: state.memory.write<TYPE>(ramstart + addr[i], value); break;
      }
   }


   //! Load an unsigned 32-bit operand
   uint32_t uLd(unsigned i) { return loadOperand<uint32_t>(i); }

   //! Load an unsigned 16-bit operand
   uint16_t uLh(unsigned i) { return loadOperand<uint16_t>(i); }

   //! Load an unsigned 8-bit operand
   uint16_t uLb(unsigned i) { return loadOperand<uint8_t>(i); }

   //! Load a signed 32-bit operand
   int32_t sLd(unsigned i) { return loadOperand<int32_t>(i); }

   //! Load a 32-bit float operand
   float fLd(unsigned index)
   {
      uint32_t bits = loadOperand<uint32_t>(index);
      float    flt;
      memcpy(&flt, &bits, sizeof(flt));
      return flt;
   }

   //! Store a 32-bit integer value
   void uSt(unsigned i, uint32_t value) { storeOperand<uint32_t>(i, value); }

   //! Store a 16-bit integer value
   void uSh(unsigned i, uint16_t value) { storeOperand<uint16_t>(i, value); }

   //! Store an 8-bit integer value
   void uSb(unsigned i, uint8_t value) { storeOperand<uint8_t>(i, value); }

   //! Store a float operand
   void fSt(unsigned i, float value)
   {
      uint32_t bits;
      memcpy(&bits, &value, sizeof(bits));
      storeOperand<uint32_t>(i, bits);
   }


   //! Relative jump
   void jump(int32_t offset)
   {
       state.branch(offset - 2);
   }

   //! Absolute jump
   void jumpabs(uint32_t address)
   {
      state.jump(address);
   }

   void call(uint32_t address, uint32_t arg)
   {
      state.stack.push32(state.getPC());
      state.stack.push32(state.frame_ptr);

      state.frame_ptr = state.stack.size();

      state.stack.push32(0); // Place holder for frame length
      state.stack.push32(0); // Place holder for local pos

      state.jump(address);
      uint8_t type = state.fetch8();

      while(true)
      {
         uint8_t local_type  = state.fetch8();
         state.stack.push8(local_type);

         uint8_t local_count = state.fetch8();
         state.stack.push8(local_count);

         if (local_count == 0) break;
      }

      // Padding to start of locals
      if ((state.stack.size() & 0b10) != 0)
      {
         state.stack.push16(0);
      }
      state.stack.write32(state.frame_ptr + 4, state.stack.size() - state.frame_ptr);

      state.stack.write32(state.frame_ptr, state.stack.size() - state.frame_ptr);

      (void) type;

   }

   void doReturn(uint32_t value)
   {
      state.stack.shrink(state.frame_ptr);
      state.frame_ptr = state.stack.pop32();
      state.jump(state.stack.pop32());
   }

   void fetchDecodeExecute()
   {
      switch(fetchOp())
      {
      case  0x00: /* nop      */ break;

      case  0x10: /* add      */ fetchA(3); uSt(2, uLd(0) + uLd(1)); break;
      case  0x11: /* sub      */ fetchA(3); uSt(2, uLd(0) - uLd(1)); break;
      case  0x12: /* mul      */ fetchA(3); uSt(2, uLd(0) * uLd(1)); break;
      case  0x13: /* div      */ fetchA(3); uSt(2, sLd(0) / sLd(1)); break;
      case  0x14: /* mod      */ fetchA(3); uSt(2, sLd(0) % sLd(1)); break;
      case  0x15: /* neg      */ fetchA(2); uSt(1, -sLd(0)); break;

      case  0x18: /* bitand   */ fetchA(3); uSt(2, uLd(0) & uLd(1)); break;
      case  0x19: /* bitor    */ fetchA(3); uSt(2, uLd(0) | uLd(1)); break;
      case  0x1A: /* bitxor   */ fetchA(3); uSt(2, uLd(0) ^ uLd(1)); break;
      case  0x1B: /* bitnot   */ fetchA(2); uSt(1, ~uLd(0)); break;
      case  0x1C: /* shiftl   */ fetchA(3); uSt(1, uLd(0) << uLd(1)); break;
      case  0x1D: /* sshiftr  */ fetchA(3); uSt(1, sLd(0) >> uLd(1)); break;
      case  0x1E: /* ushiftr  */ fetchA(3); uSt(1, uLd(0) >> uLd(1)); break;

      case  0x20: /* jump     */ fetchA(1); jump(sLd(0)); break;

      case  0x22: /* jz       */ fetchA(2); if (uLd(0) == 0)      jump(sLd(1)); break;
      case  0x23: /* jnz      */ fetchA(2); if (uLd(0) != 0)      jump(sLd(1)); break;
      case  0x24: /* jeq      */ fetchA(3); if (uLd(0) == uLd(1)) jump(sLd(2)); break;
      case  0x25: /* jne      */ fetchA(3); if (uLd(0) != uLd(1)) jump(sLd(2)); break;
      case  0x26: /* jlt      */ fetchA(3); if (sLd(0) <  sLd(1)) jump(sLd(2)); break;
      case  0x27: /* jge      */ fetchA(3); if (sLd(0) >= sLd(1)) jump(sLd(2)); break;
      case  0x28: /* jgt      */ fetchA(3); if (sLd(0) >  sLd(1)) jump(sLd(2)); break;
      case  0x29: /* jle      */ fetchA(3); if (sLd(0) <= sLd(1)) jump(sLd(2)); break;
      case  0x2A: /* jltu     */ fetchA(3); if (uLd(0) <  uLd(1)) jump(sLd(2)); break;
      case  0x2B: /* jgeu     */ fetchA(3); if (uLd(0) >= uLd(1)) jump(sLd(2)); break;
      case  0x2C: /* jgtu     */ fetchA(3); if (uLd(0) >  uLd(1)) jump(sLd(2)); break;
      case  0x2D: /* jleu     */ fetchA(3); if (uLd(0) <= uLd(1)) jump(sLd(2)); break;

      case  0x30: /* call     */ fetchA(3); call(uLd(0), uLd(1)); break;
      case  0x31: /* return   */ fetchA(1); doReturn(uLd(0)); break;
      case  0x32: /* catch    */ fetchA(2); break;
      case  0x33: /* throw    */ fetchA(2); break;
      case  0x34: /* tailcall */ fetchA(2); break;

      case  0x40: /* copy     */ fetchA(2); uSt(1, uLd(0)); break;
      case  0x41: /* copys    */ fetchA(2); uSh(1, uLh(0)); break;
      case  0x42: /* copyb    */ fetchA(2); uSb(1, uLb(0)); break;

      case  0x44: /* sexs     */ fetchA(2); break;
      case  0x45: /* sexb     */ fetchA(2); break;

      case  0x48: /* aload    */ fetchA(3); uSt(2, state.memory.read32(uLd(0) + 4 * uLd(1))); break;
      case  0x49: /* aloads   */ fetchA(3); uSt(2, state.memory.read16(uLd(0) + 4 * uLd(1))); break;
      case  0x4A: /* aloadb   */ fetchA(3); uSt(2, state.memory.read8(uLd(0) + 4 * uLd(1))); break;
      case  0x4B: /* aloadbit */ fetchA(3); break;
      case  0x4C: /* astore   */ fetchA(3); state.memory.write32(uLd(0) + 4 * uLd(1), uLd(2)); break;
      case  0x4D: /* astores  */ fetchA(3); state.memory.write16(uLd(0) + 4 * uLd(1), uLd(2)); break;
      case  0x4E: /* astoreb  */ fetchA(3); state.memory.write8(uLd(0) + 4 * uLd(1), uLd(2)); break;
      case  0x4F: /* astorebit*/ fetchA(3); break;
      case  0x50: /* stkcount */ fetchA(1); uSt(0, (state.stack.size() - local) / 4); break;
      case  0x51: /* stkpeek  */ fetchA(2); break;
      case  0x52: /* stkswap  */ break;
      case  0x53: /* stkroll  */ fetchA(2); break;
      case  0x54: /* stkcopy  */ fetchA(1); break;

      case  0x70: /* streamchr */ fetchA(1); console.write(uint8_t(uLd(1))); break;
      case  0x71: /* streamnum */ fetchA(1); break;
      case  0x72: /* streamstr */ fetchA(1); break;
      case  0x73: /* streamuch */ fetchA(1); break;

      case 0x100: /* gestalt    */ fetchA(3); break;
      case 0x101: /* debugtrap  */ fetchA(1); break;
      case 0x102: /* getmemsize */ fetchA(1); break;
      case 0x103: /* setmemsize */ fetchA(1); break;
      case 0x104: /* jumpabs    */ fetchA(1); jumpabs(uLd(0)); break;

      case 0x110: /* random     */ fetchA(2); break;
      case 0x111: /* setrandom  */ fetchA(1); state.random.predictableSeed(uLd(0)); break;

      case 0x120: /* quit       */ state.quit(); break;
      case 0x121: /* verify     */ fetchA(1); uSt(0, 0); break;
      case 0x122: /* restart    */ state.reset(); break;
      case 0x123: /* save       */ fetchA(2); uSt(1, state.save() ? 0 : 1); break;
      case 0x124: /* restore    */ fetchA(2); uSt(1, state.restore() ? 0 : 1); break;
      case 0x125: /* saveundo   */ fetchA(1); uSt(0, state.saveUndo() ? 0 : 1); break;
      case 0x126: /* restoreundo*/ fetchA(1); uSt(0, state.restoreUndo() ? 0 : 1); break;
      case 0x127: /* protect    */ fetchA(2); break;

      case 0x130: /* glk        */ fetchA(3); break;

      case 0x140: /* getstrtbl  */ fetchA(1); break;
      case 0x141: /* setstrtbl  */ fetchA(1); break;

      case 0x148: /* getiosys   */ fetchA(1); break;
      case 0x149: /* setiosys   */ fetchA(1); break;
      case 0x150: /* linsearch  */ fetchA(8); break;
      case 0x151: /* binsearch  */ fetchA(8); break;
      case 0x152: /* lnksearch  */ fetchA(7); break;

      case 0x160: /* callf      */ fetchA(2); call(uLd(0), uLd(1)); break;
      case 0x161: /* callfi     */ fetchA(3); call(uLd(0), uLd(1)); break;
      case 0x162: /* callfii    */ fetchA(4); call(uLd(0), uLd(1)); break;
      case 0x163: /* callfiii   */ fetchA(5); call(uLd(0), uLd(1)); break;

      case 0x170: /* mzero      */ fetchA(2); break;
      case 0x171: /* mcopy      */ fetchA(3); break;
      case 0x172: /* malloc     */ fetchA(2); break;
      case 0x173: /* mfree      */ fetchA(1); break;

      case 0x180: /* accelfunc  */ fetchA(2); break;
      case 0x181: /* accelparam */ fetchA(2); break;

      case 0x190: /* numtof     */ fetchA(2); fSt(1, sLd(0)); break;
      case 0x191: /* ftonumz    */ fetchA(2); uSt(1, fLd(0)); break; // TODO rounding
      case 0x192: /* ftonumn    */ fetchA(2); uSt(1, fLd(0)); break; // TODO rounding

      case 0x198: /* ceil       */ fetchA(2); fSt(1, ceil(fLd(0))); break;
      case 0x199: /* floor      */ fetchA(2); fSt(1, floor(fLd(0))); break;

      case 0x1A0: /* fadd       */ fetchA(3); fSt(2, fLd(0) + fLd(1)); break;
      case 0x1A1: /* fsub       */ fetchA(3); fSt(2, fLd(0) - fLd(1)); break;
      case 0x1A2: /* fmul       */ fetchA(3); fSt(2, fLd(0) * fLd(1)); break;
      case 0x1A3: /* fdiv       */ fetchA(3); fSt(2, fLd(0) / fLd(1)); break;
      case 0x1A4: /* fmod       */ fetchA(4); break;

      case 0x1A8: /* sqrt       */ fetchA(2); fSt(1, sqrt(fLd(0))); break;
      case 0x1A9: /* exp        */ fetchA(2); fSt(1, exp(fLd(0))); break;
      case 0x1AA: /* log        */ fetchA(2); fSt(1, log(fLd(0))); break;
      case 0x1AB: /* pow        */ fetchA(3); fSt(2, pow(fLd(0), fLd(1))); break;

      case 0x1B0: /* sin        */ fetchA(2); fSt(1, sin(fLd(0))); break;
      case 0x1B1: /* cos        */ fetchA(2); fSt(1, cos(fLd(0))); break;
      case 0x1B2: /* tan        */ fetchA(2); fSt(1, tan(fLd(0))); break;
      case 0x1B3: /* asin       */ fetchA(2); fSt(1, asin(fLd(0))); break;
      case 0x1B4: /* acos       */ fetchA(2); fSt(1, acos(fLd(0))); break;
      case 0x1B5: /* atan       */ fetchA(2); fSt(1, atan(fLd(0))); break;
      case 0x1B6: /* atan2      */ fetchA(3); fSt(2, atan2(fLd(0), fLd(1))); break;

      case 0x1C0: /* jfeq       */ fetchA(3); if (fLd(0) == fLd(1)) jump(sLd(2)); break;
      case 0x1C1: /* jfne       */ fetchA(3); if (fLd(0) != fLd(1)) jump(sLd(2)); break;
      case 0x1C2: /* jflt       */ fetchA(3); if (fLd(0) <  fLd(1)) jump(sLd(2)); break;
      case 0x1C3: /* jfle       */ fetchA(3); if (fLd(0) <= fLd(1)) jump(sLd(2)); break;
      case 0x1C4: /* jfgt       */ fetchA(3); if (fLd(0) >  fLd(1)) jump(sLd(2)); break;
      case 0x1C5: /* jfge       */ fetchA(3); if (fLd(0) >= fLd(1)) jump(sLd(2)); break;

      case 0x1C8: /* jisnan     */ fetchA(2); if (std::isnan(fLd(0))) jump(sLd(1)); break;
      case 0x1C9: /* jisinf     */ fetchA(2); if (std::isinf(fLd(0))) jump(sLd(1)); break;

      default:
         throw "unimplemented op";
         break;
      }
   }
};

} // namespace Glulx

