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

#ifndef ZSTORY_H
#define ZSTORY_H

#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>

#include "share/StoryBase.h"

#include "ZHeader.h"

namespace Z {

//! Manage Z story image
class Story : public StoryBase<ZHeader>
{
public:
   Story() = default;

private:
   virtual bool checkHeader(FILE* fp) override
   {
      uint8_t version;

      if (fread(&version, 1, 1, fp) == 1)
      {
         return (version >= 1) && (version <= 8);
      }

      return false;
   }

   virtual bool validateHeader(FILE* fp, size_t& size) override
   {
      const ZHeader* header = getHeader();

      if (!header->isVersionValid())
      {
         error = "Unexpected Z version ";
         error += std::to_string(header->version);
         return false;
      }

      if (header->getStorySize() == 0)
      {
         // Some older Z files had a zerp file size in the header
         if (fseek(fp, 0, SEEK_END) == 0)
         {
            long file_size = ftell(fp);
            if (file_size > 0)
            {
               if (fseek(fp, sizeof(ZHeader), SEEK_SET) == 0)
               {
                  getHeader()->setStorySize(file_size);
               }
            }
         }
      }

      if (header->getStorySize() == 0)
      {
         error = "Failed to find file size";
         return false;
      }

      size = header->getStorySize();

      return true;
   }

   virtual bool validateImage() const override
   {
      const ZHeader* header = getHeader();

      uint16_t checksum = 0;

      for(uint32_t i = BODY_START; i < header->getStorySize(); ++i)
      {
         checksum += image[i];
      }

      return header->checksum == checksum;
   }
};

} // namespace Z

#endif
