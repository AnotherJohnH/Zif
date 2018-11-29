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

class ZQuetzal
{
public:
   ZQuetzal(const ZStory& story_)
      : story(story_)
   {
   }

   //! Write machine state to a Quetzal file
   bool write(const std::string& filename,
              uint32_t           pc,
              const ZMemory&     memory,
              const ZStack&      stack)
   {
      STB::IFF::Document doc{"FORM", "IFSZ"};

      writeHeader(          doc, pc);
      writeCompressedMemory(doc, memory);
      writeStacks(          doc, stack);

      return doc.write(filename);
   }

   //! Read machine state from a Quetzal file
   bool read(const std::string& filename,
             uint32_t&          pc,
             ZMemory&           memory,
             ZStack&            stack,
             std::string        error)
   {
       STB::IFF::Document doc;
       if (!doc.read(filename))
       {
          error = "Failed to open file '";
          error += filename;
          error += "'";
       }

       if (!doc.isDocType("FORM") || !doc.isFileType("IFSZ"))
       {
          error = "File is not an IFF FORM of type IFSZ";
          return false;
       }

       return readHeader(doc, pc, error) &&
              readMemory(doc, memory, error) &&
              readStacks(doc, stack, error);
   }

private:
   struct IFhd
   {
      STB::Big16 release;
      uint8_t    serial[6];
      STB::Big16 checksum;
      uint8_t    initial_pc[3];
   };

   const ZStory& story;

   //! Prepare IFhd chunk
   void writeHeader(STB::IFF::Document& doc,
                    uint32_t            pc)
   {
      STB::IFF::Chunk& ifhd_chunk = doc.addChunk("IFhd");
      const ZHeader*   header     = story.getHeader();
      IFhd             ifhd;

      ifhd.release       = header->release;
      memcpy(ifhd.serial, header->serial, 6);
      ifhd.checksum      = header->checksum;
      ifhd.initial_pc[0] = pc >> 16;
      ifhd.initial_pc[1] = pc >> 8;
      ifhd.initial_pc[2] = pc;

      ifhd_chunk.push(&ifhd, 13);
   }

   //! Prepare CMem chunk
   void writeCompressedMemory(STB::IFF::Document& doc,
                              const ZMemory&      memory)
   {
      STB::IFF::Chunk& cmem = doc.addChunk("CMem");

      const uint8_t* ref = story.getGame() - sizeof(ZHeader);
      const uint8_t* mem = &memory[0];

      uint8_t run_length = 0;
      for(uint32_t i=0; i<memory.size(); i++)
      {
         uint8_t enc_byte;
         if (i < story.getGameSize())
         {
            enc_byte = ref[i] ^ mem[i];
         }
         else
         {
            enc_byte = mem[i];
         }
         if (enc_byte == 0x00)
         {
            if (++run_length == 0)
            {
               cmem.push(uint8_t(0x00));
               cmem.push(uint8_t(0xFF));
            }
         }
         else
         {
            if (run_length != 0)
            {
               cmem.push(uint8_t(0x00));
               cmem.push(uint8_t(run_length-1));
               run_length = 0;
            }
            cmem.push(enc_byte);
         }
      }
   }

   //! Prepare Stks chunk
   void writeStacks(STB::IFF::Document& doc,
                    const ZStack&       stack)
   {
      // Stacks (store as Big endian)
      STB::IFF::Chunk& stks = doc.addChunk("Stks", stack.size());
      for(uint16_t i=0; i<stack.size(); i++)
      {
         STB::Big16 word = stack[i];
         stks.push(word);
      }
   }

   //! Read and decode IFhd chunk
   bool readHeader(STB::IFF::Document& doc,
                   uint32_t&           pc,
                   std::string         error)
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
   bool readMemory(STB::IFF::Document& doc,
                   ZMemory&            memory,
                   std::string         error)
   {
      uint32_t size = 0;
      uint8_t* mem  = &memory[0];

      const uint8_t* cmem = doc.load<uint8_t>("CMem", &size);
      if (cmem != nullptr)
      {
         const uint8_t* ref  = story.getGame() - sizeof(ZHeader);
         uint32_t       addr = 0;

         for(uint32_t i=0; i<size; )
         {
            uint8_t byte = cmem[i++];
            if (byte == 0)
            {
               if (i == size)
               {
                  error = "Incomplete CMem chunk";
                  return false;
               }
               unsigned n = cmem[i++] + 1;
               for(unsigned j=0; j<n; j++)
               {
                  if (!decodeByte(memory, ref, addr++, 0))
                  {
                     error = "CMem chunk too big";
                     return false;
                  }
               }
            }
            else
            {
               if (!decodeByte(memory, ref, addr++, byte))
               {
                  error = "CMem chunk too big";
                  return false;
               }
            }
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

   bool decodeByte(ZMemory& memory, const uint8_t* ref, uint32_t addr, uint8_t byte)
   {
      if (addr >= memory.size())
      {
         return false;
      }

      if (addr < story.getGameSize())
      {
         memory[addr] = ref[addr] ^ byte;
      }
      else
      {
         memory[addr] = byte;
      }

      return true;
   }

   //! Read and decode Stks chunk
   bool readStacks(STB::IFF::Document& doc,
                   ZStack&             stack,
                   std::string         error)
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
