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

#ifndef GLULX_STORY_H
#define GLULX_STORY_H

#include "share/StoryBase.h"

#include "Glulx/Header.h"

namespace Glulx {

//! Manage Glulx story image
class Story : public StoryBase<Glulx::Header>
{
public:
   Story() = default;

   virtual std::string getBlorbId() const override { return "GLUL"; }

   virtual bool validateHeader(FILE* fp, size_t& file_size) override
   {
      const Header* header = getHeader();

      if ((header->magic[0] != 'G') ||
          (header->magic[0] != 'l') ||
          (header->magic[0] != 'u') ||
          (header->magic[0] != 'l'))
      {
         error = "Invalid Glulx magic key";
         return false;
      }

      file_size = header->ext_start;

      return true;
   }

   virtual bool validateImage() const override
   {
      const Header* header = getHeader();

      uint32_t checksum = 0;

      for(size_t i=0; i<header->ext_start; i+= 4)
      {
         const uint32_t& word = (const uint32_t&) image[i];
         // XXX assuming the host machine is little endian
         checksum += STB::endianSwap(word);
      }

      return header->checksum == checksum;
   }
};

} // namespace Glulx

#endif
