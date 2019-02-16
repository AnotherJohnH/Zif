//------------------------------------------------------------------------------
// Copyright (c) 2018 John D. Haughton
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

#ifndef ZQUETZAL_H
#define ZQUETZAL_H

#include <cstring>

#include "STB/IFF.h"

#include "share/Story.h"
#include "share/State.h"

//! Encode/decode game state to/from quetzal format
class ZQuetzal
{
public:
   ZQuetzal() = default;

   //! Get error message for the last error
   const std::string& getLastError() const { return error; }

   //! Save the Machine state in this Quetzal object
   void encode(const IF::Story&  story,
               const IF::State&  state)
   {
      story.encodeQuetzalHeader(doc, state.getPC());

      encodeMemory(story, state.memory);
      encodeStacks(state.stack);
      encodeZifHeader(state.random);
   }

   //! Restore the ZMachine state from this Quetzal object
   bool decode(const IF::Story& story,
               IF::State&       state)
   {
      decodeZifHeader(state.random);

      IF::Memory::Address pc;
      if (!story.decodeQuetzalHeader(doc, pc))
      {
         error = story.getLastError();
         return false;
      }

      state.jump(pc);

      error = "";
      return decodeMemory(story, state.memory) &&
             decodeStacks(state.stack);
   }

   //! Write Quetzal object to a file
   bool write(const std::string& path)
   {
      return doc.write(path);
   }

   //! Read Quetzal object from a file
   bool read(const std::string& path)
   {
      error = "";

      if (!doc.read(path, "FORM", "IFZS"))
      {
         error = "Failed to open Quetzal save file '";
         error += path;
         error += "'";
         return false;
      }

      return true;
   }

private:
   struct ZifHeader
   {
      STB::Big64 rand_num_state;
   };

   STB::IFF::Document doc{"FORM", "IFZS"};
   std::string        path{};
   std::string        error{};

   //! Prepare ZifH chunk
   void encodeZifHeader(const IF::Random& random)
   {
      STB::IFF::Chunk* zifh_chunk = doc.newChunk("ZifH", sizeof(ZifHeader));
      ZifHeader        zifh;

      zifh.rand_num_state = random.internalState();

      zifh_chunk->push(zifh);
   }

   //! Prepare CMem chunk
   void encodeMemory(const IF::Story&  story, const IF::Memory& memory)
   {
      STB::IFF::Chunk* cmem = doc.newChunk("CMem");

      const uint8_t* ref = story.data();
      const uint8_t* mem = memory.data();

      uint32_t run_length = 0;
      for(uint32_t i=0; i<=memory.getWriteEnd(); i++)
      {
         uint8_t enc_byte = i < story.size() ? ref[i] ^ mem[i]
                                             : mem[i];
         if (enc_byte == 0x00)
         {
            ++run_length;
         }
         else
         {
            while(run_length != 0)
            {
               uint32_t n = run_length <= 0x100 ? run_length
                                                : 0x100;

               cmem->push(uint8_t(0x00));
               cmem->push(uint8_t(n - 1));

               run_length -= n;
            }
            cmem->push(enc_byte);
         }
      }
   }

   //! Prepare Stks chunk
   void encodeStacks(const IF::Stack& stack)
   {
      // Stacks (store as Big endian)
      STB::IFF::Chunk* stks = doc.newChunk("Stks");
      stks->push(stack.data(), stack.size());
   }

   //! Decode ZifH chunk
   void decodeZifHeader(IF::Random& random)
   {
       const ZifHeader* zifh = doc.load<ZifHeader>("ZifH");
       if (zifh == nullptr)
       {
          // header is optional
          return;
       }

       random.internalState() = zifh->rand_num_state;
   }

   //! Read and decode CMem or UMem chunk
   bool decodeMemory(const IF::Story& story, IF::Memory& memory)
   {
      uint32_t size = 0;

      const uint8_t* cmem = doc.load<uint8_t>("CMem", &size);
      if (cmem != nullptr)
      {
         const uint8_t* ref  = story.data();
         uint32_t       addr = 0;

         for(uint32_t i=0; i<size; )
         {
            uint8_t byte = cmem[i++];
            if (byte == 0)
            {
               if (i == size)
               {
                  printf("i=%x\n", i);
                  error = "Incomplete CMem chunk";
                  return false;
               }
               unsigned n = cmem[i++] + 1;
               for(unsigned j=0; j<n; j++)
               {
                  if (!decodeByte(memory, ref, story.size(), addr++, 0))
                  {
                     error = "CMem chunk too big";
                     return false;
                  }
               }
            }
            else
            {
               if (!decodeByte(memory, ref, story.size(), addr++, byte))
               {
                  error = "CMem chunk too big";
                  return false;
               }
            }
         }

         while(addr < story.size())
         {
            (void) decodeByte(memory, ref, story.size(), addr++, 0x00);
         }

         return true;
      }

      const uint8_t* umem = doc.load<uint8_t>("UMem", &size);
      if (umem != nullptr)
      {
         for(uint32_t addr=0; addr<size; addr++)
         {
            memory.set8(addr, umem[addr]);
         }
         return true;
      }

      error = "CMem or UMemchunk not found";

      return false;
   }

   bool decodeByte(IF::Memory& memory,
                   const uint8_t* ref,
                   uint32_t       ref_size,
                   uint32_t       addr,
                   uint8_t        byte)
   {
      if (addr >= memory.size())
      {
         return false;
      }

      if (addr < ref_size)
      {
         memory.set8(addr, ref[addr] ^ byte);
      }
      else
      {
         memory.set8(addr, byte);
      }

      return true;
   }

   //! Read and decode Stks chunk
   bool decodeStacks(IF::Stack& stack)
   {
       uint32_t stack_size = 0;
       const uint8_t* bytes = doc.load<uint8_t>("Stks", &stack_size);
       if (bytes == nullptr)
       {
          error = "Stks chunk not found";
       }

       stack.clear();
       for(uint32_t i=0; i<stack_size; i++)
       {
          stack.push8(bytes[i]);
       }

       return true;
   }
};

#endif
