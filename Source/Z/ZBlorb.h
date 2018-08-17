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

#ifndef ZBLORB_H
#define ZBLORB_H

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "PLT/File.h"
#include "STB/Endian.h"

class ZBlorb
{
public:
   ZBlorb() = default;

   bool init(const char* filename)
   {
      PLT::File file(nullptr, filename);
      if(!file.openForRead()) return false;

      char       id[4];
      STB::Big32 length;

      // Confirm FORM format
      if (!file.read(id, 4)) return false;
      if (strncmp(id, "FORM", 4) != 0) return false;

      // Read FORM size
      if (!file.read(&length, 4)) return false;
      form_size = length;

      // Confirm FORM type is IFRS
      if (!file.read(id, 4)) return false;
      if (strncmp(id, "IFRS", 4) != 0) return false;

      // Confirm first chunk type is a resource index
      if (!file.read(id, 4)) return false;
      if (strncmp(id, "RIdx", 4) != 0) return false;

      // Read resource index size
      if (!file.read(&length, 4)) return false;
      uint32_t chunk_length = length;
      uint32_t num_entries = (chunk_length - 4) / 12;

      for(uint32_t i=0; i<num_entries; i++)
      {
      }

      return false;
   }

   unsigned offsetOf(const char* chunk)
   {
       return 0;
   }

private:
   uint32_t form_size{0};
};

#endif
