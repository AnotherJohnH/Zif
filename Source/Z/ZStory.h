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

//! Manage Z story image
class ZStory : public StoryBase
{
public:
   ZStory() = default;

   //! Return pointer to initial state of Z header
   const ZHeader* getHeader() const { return reinterpret_cast<const ZHeader*>(data()); }

private:
   static const uint32_t GAME_START = sizeof(ZHeader);

   //! Return pointer to initial state of Z header
   ZHeader* getHeader() { return reinterpret_cast<ZHeader*>(image.data()); }

   virtual std::string getBlorbId() const override { return "ZCOD"; }

   virtual bool loadImage(FILE* fp) override
   {
      image.resize(sizeof(ZHeader));

      if (fread(image.data(), sizeof(ZHeader), 1, fp) != 1)
      {
         error = "Failed to read Z header";
         return false;
      }

      const ZHeader* header = getHeader();

      if (!header->isVersionValid())
      {
         error = "Unexpected Z version ";
         error += std::to_string(header->version);
         return false;
      }

      if (header->getStorySize() == 0)
      {
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

      image.resize(header->getStorySize());

      if (fread(&image[GAME_START], image.size() - GAME_START, 1, fp) != 1)
      {
         error = "Failed to read Z body";
         return false;
      }

      return true;
   }

   virtual void validateImage() override
   {
      const ZHeader* header = getHeader();

      uint16_t checksum = 0;

      for(uint32_t i = GAME_START; i < header->getStorySize(); ++i)
      {
         checksum += image[i];
      }

      is_valid = checksum == header->checksum;
   }
};

#endif
