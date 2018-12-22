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

#include "ZStory.h"
#include "ZStack.h"
#include "ZMemory.h"

//! Encode/decode game state to/from quetzal format
class ZQuetzal
{
public:
   ZQuetzal() = default;

   //! Get error message for the last error
   const std::string& getLastError() const { return error; }

   //! Save the ZMachine state in this Quetzal object
   void encode(const ZStory&  story,
               uint32_t       pc,
               uint32_t       rand_num_state,
               const ZMemory& memory,
               const ZStack&  stack)
   {
      encodeHeader(story, pc);
      encodeMemory(story, memory);
      encodeStacks(stack);
      encodeZifHeader(rand_num_state);
   }

   //! Restore the ZMachine state from this Quetzal object
   bool decode(const ZStory& story,
               uint32_t&     pc,
               uint32_t&     rand_num_state,
               ZMemory&      memory,
               ZStack&       stack)
   {
      error = "";

      decodeZifHeader(rand_num_state);

      return decodeHeader(story, pc) &&
             decodeMemory(story, memory) &&
             decodeStacks(stack);
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

      if (!doc.read(path))
      {
         error = "Failed to open Quetzal save file '";
         error += path;
         error += "'";
         return false;
      }

      if (!doc.isDocType("FORM") || !doc.isFileType("IFZS"))
      {
         error = "File is not an IFF FORM of type IFZS";
         return false;
      }

      return true;
   }

private:
   struct ZifHeader
   {
      STB::Big32 rand_num_state;
   };

   struct IFhd
   {
      STB::Big16 release;
      uint8_t    serial[6];
      STB::Big16 checksum;
      uint8_t    initial_pc[3];
   };

   STB::IFF::Document doc{"FORM", "IFZS"};
   std::string        path{};
   std::string        error{};

   //! Prepare ZifH chunk
   void encodeZifHeader(uint32_t rand_num_state)
   {
      STB::IFF::Chunk* zifh_chunk = doc.newChunk("ZifH", sizeof(ZifHeader));
      ZifHeader        zifh;

      zifh.rand_num_state = rand_num_state;

      zifh_chunk->push(zifh);
   }

   //! Prepare IFhd chunk
   void encodeHeader(const ZStory& story, uint32_t pc)
   {
      STB::IFF::Chunk* ifhd_chunk = doc.newChunk("IFhd", 13);
      const ZHeader*   header     = story.getHeader();
      IFhd             ifhd;

      ifhd.release       = header->release;
      memcpy(ifhd.serial, header->serial, 6);
      ifhd.checksum      = header->checksum;
      ifhd.initial_pc[0] = pc >> 16;
      ifhd.initial_pc[1] = pc >> 8;
      ifhd.initial_pc[2] = pc;

      ifhd_chunk->push(&ifhd, 13);
   }

   //! Prepare CMem chunk
   void encodeMemory(const ZStory&  story, const ZMemory& memory)
   {
      STB::IFF::Chunk* cmem = doc.newChunk("CMem");

      const uint8_t* ref = story.data();
      const uint8_t* mem = memory.getData();

      uint32_t run_length = 0;
      for(uint32_t i=0; i<memory.getStaticAddr(); i++)
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
   void encodeStacks(const ZStack& stack)
   {
      // Stacks (store as Big endian)
      STB::IFF::Chunk* stks = doc.newChunk("Stks", stack.size() * 2);
      for(uint16_t i=0; i<stack.size(); i++)
      {
         STB::Big16 word = stack[i];
         stks->push(word);
      }
   }

   //! Decode ZifH chunk
   void decodeZifHeader(uint32_t& rand_num_state)
   {
       const ZifHeader* zifh = doc.load<ZifHeader>("ZifH");
       if (zifh == nullptr)
       {
          // header is optional
          return;
       }

       rand_num_state = zifh->rand_num_state;
   }

   //! Decode IFhd chunk
   bool decodeHeader(const ZStory& story, uint32_t& pc)
   {
       const IFhd* ifhd = doc.load<IFhd>("IFhd");
       if (ifhd == nullptr)
       {
          error = "IFhd chunk not found";
          return false;
       }

       const ZHeader* header = story.getHeader();

       // Verify story version matches
       if ((ifhd->release != header->release) ||
           (memcmp(ifhd->serial, header->serial, 6) != 0) ||
           (ifhd->checksum != header->checksum))
       {
          error = "IFhd mismatch";
          return false;
       }

       // Extract PC
       pc = (ifhd->initial_pc[0]<<16) |
            (ifhd->initial_pc[1]<<8)  |
             ifhd->initial_pc[2];

       return true;
   }

   //! Read and decode CMem or UMem chunk
   bool decodeMemory(const ZStory& story, ZMemory& memory)
   {
      uint32_t size = 0;
      uint8_t* mem  = memory.getData();

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
         memcpy(mem, umem, size);
         return true;
      }

      error = "CMem or UMemchunk not found";

      return false;
   }

   bool decodeByte(ZMemory&       memory,
                   const uint8_t* ref,
                   uint32_t       ref_size,
                   uint32_t       addr,
                   uint8_t        byte)
   {
      if (addr >= memory.getSize())
      {
         return false;
      }

      if (addr < ref_size)
      {
         memory.set(addr, ref[addr] ^ byte);
      }
      else
      {
         memory.set(addr, byte);
      }

      return true;
   }

   //! Read and decode Stks chunk
   bool decodeStacks(ZStack& stack)
   {
       uint32_t stack_size = 0;
       const STB::Big16* word = doc.load<STB::Big16>("Stks", &stack_size);
       if (word == nullptr)
       {
          error = "Stks chunk not found";
       }

       stack.clear();
       for(uint32_t i=0; i<(stack_size/2); i++)
       {
          stack.push_back(word[i]);
       }

       return true;
   }
};

#endif
