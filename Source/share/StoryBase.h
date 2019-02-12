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

#ifndef STORY_BASE_H
#define STORY_BASE_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "Blorb.h"

//! Base class for story image objects
class StoryBase
{
public:
   StoryBase() = default;

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
      error    = "";
   }

   //! Load story from file
   bool load(const std::string& path)
   {
      clear();

      bool ok = false;

      FILE* fp = fopen(path.c_str(), "r");
      if (fp == nullptr)
      {
         error = "Failed to open story file \'";
         error += path;
         error += "\'";
      }
      else
      {
         if (!seekToHeader(fp, path))
         {
            error = "Failed to seek to header";
         }
         else
         {
            if (loadImage(fp))
            {
               validateImage();
               extractFilename(path);
               ok = true;
            }
         }

         fclose(fp);
      }

      if (!ok)
      {
         clear();
      }

      return ok;
   }

protected:
   std::vector<uint8_t> image;
   bool                 is_valid{false};
   std::string          filename{};
   std::string          error{};

   //! Get image Blorb Id 
   virtual std::string getBlorbId() const = 0;

   //! Check loaded the story image
   virtual bool loadImage(FILE* fp) = 0;

   //! Check loaded image matches the checksum in the header
   virtual void validateImage() = 0;

   bool seekToHeader(FILE* fp, const std::string& path)
   {
      if (path.find("blorb") != std::string::npos)
      {
         Blorb       blorb{};
         std::string type;
         uint32_t    offset{0};

         if (blorb.findResource(path, Blorb::Resource::EXEC, /* index */ 0, type, offset) &&
             (type == getBlorbId()))
         {
            return fseek(fp, offset, SEEK_SET) == 0;
         }
      }

      return true;
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
