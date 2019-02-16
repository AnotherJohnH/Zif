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

#ifndef Z_MEMORY_H
#define Z_MEMORY_H

#include <cstring>

#include "share/Memory.h"

#include "Z/Story.h"

//! Z machine memory
class ZMemory : public IF::Memory
{
public:
   ZMemory() = default;

   //! Get writable pointer to header
   ZHeader* getHeader() { return reinterpret_cast<ZHeader*>(raw.data()); }

   //! Configure memory for a game
   bool init(const Z::Story& story_)
   {
      const ZHeader* header = story_.getHeader();

      resize(header->getMemoryLimit());

      limitWrite(0, header->stat);

      memcpy(raw.data(), header, sizeof(ZHeader));

      return true;
   }

   //! Reset memory from game image
   void reset(const IF::Story& story)
   {
      // TODO the header should be reset (only bits 0 and 1 from Flags 2
      //      shoud be preserved)

      memcpy(raw.data() + sizeof(ZHeader),
             story.data() + sizeof(ZHeader),
             story.size() - sizeof(ZHeader));
   }
};

#endif
