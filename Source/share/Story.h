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

#ifndef IF_STORY_H
#define IF_STORY_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "STB/IFF.h"

#include "share/Memory.h"

namespace IF {

//! Story base class for an interactive fiction VM
class Story
{
public:
   Story() = default;

   //! Get error message for the last error
   const std::string& getLastError() const { return error; }

   //! Return true if previous load() was successful
   bool isLoadedOk() const { return !image.empty(); }

   //! Return true if previous load() was successful and validates ok
   bool isValid() const { return is_valid; }

   //! Return name of ZStory file from previous succesful load()
   const std::string& getFilename() const { return filename; }

   //! Return pointer to initial state of Z story
   const uint8_t* data() const { return image.data(); }

   //! Return size of story (bytes)
   size_t size() const { return image.size(); }

   //! Clear all state from any previously loaded image
   void clear()
   {
      image.clear();

      is_valid = false;
      filename = "";
   }

   bool isRecognised(const std::string& path)
   {
      FILE* fp = fopen(path.c_str(), "r");
      if (fp == nullptr)
      {
         return false;
      }

      bool recognised = checkHeader(fp);

      fclose(fp);

      return recognised;
   }

   //! Load story from file
   bool load(const std::string& path, size_t offset = 0)
   {
      clear();

      FILE* fp = fopen(path.c_str(), "r");
      if (fp == nullptr)
      {
         error = "Failed to open story file \'";
         error += path;
         error += "\'";
         return false;
      }

      bool ok = false;

      if (fseek(fp, offset, SEEK_SET) != 0)
      {
         error = "Failed to seek to header";
      }
      else
      {
         image.resize(getSizeOfHeader());

         if (fread(image.data(), getSizeOfHeader(), 1, fp) != 1)
         {
            error = "Failed to read header";
         }
         else
         {
            size_t file_size;

            if (validateHeader(fp, file_size))
            {
               image.resize(file_size);

               if (fread(&image[getSizeOfHeader()], file_size - getSizeOfHeader(), 1, fp) != 1)
               {
                  error = "Failed to read body";
               }
               else
               {
                  is_valid = validateImage();
                  extractFilename(path);
                  ok = true;
               }
            }
         }
      }

      fclose(fp);

      if (!ok)
      {
         clear();
      }

      return ok;
   }

   //! Validate loaded image 
   virtual bool validateImage() const = 0;

   //! Get size of header (bytes)
   virtual size_t getSizeOfHeader() const  = 0;

   //! Check header is the right format
   virtual bool checkHeader(FILE* fp) = 0;

   //! Validate header and return story size
   virtual bool validateHeader(FILE* fp, size_t& size) = 0;

   //! Prepare VM memory for this Z-story image
   virtual void prepareMemory(IF::Memory& memory) const = 0;

   //! Reset VM memory for this Z-story image
   virtual void resetMemory(IF::Memory& memory) const = 0;

   //! Encode Quetzal header chunk 
   virtual void encodeQuetzalHeader(STB::IFF::Document& doc, uint32_t pc) const = 0;

   //! Decode Quetzal header chunk 
   virtual bool decodeQuetzalHeader(STB::IFF::Document& doc, uint32_t& pc) const = 0;

protected:
   std::vector<uint8_t> image;
   mutable std::string  error{};

private:
   bool        is_valid{false};
   std::string filename{};

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

//! Base class for story image objects
template <typename HEADER>
class StoryBase : public Story
{
public:
   //! Get size of header (bytes)
   virtual size_t getSizeOfHeader() const override { return sizeof(HEADER); }

   //! Return pointer to initial state of header
   const HEADER* getHeader() const { return reinterpret_cast<const HEADER*>(image.data()); }

protected:
   //! Return pointer to header
   HEADER* getHeader() { return reinterpret_cast<HEADER*>(image.data()); }
};

} // namespace IF

#endif
