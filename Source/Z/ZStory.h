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

#include "ZHeader.h"
#include "ZBlorb.h"

//! Manage Z story image
class ZStory
{
public:
   ZStory() = default;

   //! Get error message for the last error
   const std::string& getLastError() const { return error; }

   //! Return true if previous load() was successful
   bool isLoadedOk() const { return !image.empty(); }

   //! Return true if previous load() was successful and checksum matches
   bool isChecksumOk() const { return checksum_ok; }

   //! Return name of ZStory file from previous succesful load()
   const std::string& getFilename() const { return filename; }

   //! Return pointer to initial state of Z header
   const ZHeader* getHeader() const { return reinterpret_cast<const ZHeader*>(data()); }

   //! Return pointer to initial state of Z story
   const uint8_t* data() const { return image.data(); }

   //! Return size of story (bytes)
   size_t size() const { return image.size(); }

   bool load(const std::string& path)
   {
      error = "";

      bool ok = false;

      FILE* fp = fopen(path.c_str(), "r");
      if (fp == nullptr)
      {
         error = "Failed to open story Z-file \'";
         error += path;
         error += "\'";
      }
      else
      {
         if (!seekToZHeader(fp, path))
         {
            error = "Failed to seek to Z header";
         }
         else
         {
            image.resize(sizeof(ZHeader));

            if (fread(image.data(), sizeof(ZHeader), 1, fp) != 1)
            {
               error = "Failed to read Z header";
            }
            else
            {
               const ZHeader* header = getHeader();

               if (!header->isVersionValid())
               {
                  error = "Unexpected Z version ";
                  error += std::to_string(header->version);
               }
               else
               {
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
                  }
                  else
                  {
                     image.resize(header->getStorySize());

                     if (fread(&image[GAME_START], image.size() - GAME_START, 1, fp) != 1)
                     {
                        error = "Failed to read Z body";
                     }
                     else
                     {
                        calcCheckSum();
                        extractFilename(path);
                        ok = true;
                     }
                  }
               }
            }
         }

         fclose(fp);
      }

      if (!ok)
      {
         image.clear();
         checksum_ok = false;
         filename = "";
      }

      return ok;
   }

private:
   static const uint32_t GAME_START = sizeof(ZHeader);

   std::vector<uint8_t> image;
   bool                 checksum_ok{false};
   std::string          filename{};
   std::string          error{};

   //! Return pointer to initial state of Z header
   ZHeader* getHeader() { return reinterpret_cast<ZHeader*>(image.data()); }

   bool seekToZHeader(FILE* fp, const std::string& path)
   {
      if (path.find(".zblorb") != std::string::npos)
      {
         ZBlorb      zblorb{};
         std::string type;
         uint32_t    offset{0};

         if (zblorb.findResource(path, ZBlorb::Resource::EXEC, /* index */ 0, type, offset) &&
             (type == "ZCOD"))
         {
            return fseek(fp, offset, SEEK_SET) == 0;
         }
      }

      return true;
   }

   void calcCheckSum()
   {
      const ZHeader* header = getHeader();

      uint16_t checksum = 0;

      for(uint32_t i = GAME_START; i < header->getStorySize(); ++i)
      {
         checksum += image[i];
      }

      checksum_ok = checksum == header->checksum;
   }

   void extractFilename(const std::string& path)
   {
      size_t slash = path.rfind('/');
      if (slash == std::string::npos)
      {
         filename = path;
      }
      else
      {
         filename = path.substr(slash + 1);
      }
   }
};

#endif
