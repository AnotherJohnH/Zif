//------------------------------------------------------------------------------
// Copyright (c) 2016-2018 John D. Haughton
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

#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <string>

//! Log file
class Log
{
public:
   Log(const std::string& filename_)
      : filename(filename_)
   {
   }

   ~Log()
   {
      if (fp != nullptr) ::fclose(fp);
   }

   //! write a single character to the log
   void write(char ch)
   {
      ensureOpen();

      if (suffix != "")
      {
         // partial write in progress => end it
         for(const auto& ch : suffix)
         {
            ::fputc(ch, fp);
         }

         prefix = "";
         suffix = "";
      }

      ::fputc(ch, fp);
   }

   //! write a string to the log
   void write(const std::string& str)
   {
      for(const auto& ch : str)
      {
         write(ch);
      }
   }

   void writePart(const std::string& prefix_, char ch, const std::string& suffix_)
   {
      ensureOpen();

      if (prefix != prefix_)
      {
         // start a new partial write
         write(prefix_);
         prefix = prefix_;
         suffix = suffix_;
      }

      ::fputc(ch, fp);
   }

private:
   std::string filename;
   FILE*       fp{nullptr};
   std::string prefix;
   std::string suffix;

   void ensureOpen()
   {
      if(fp == nullptr) fp = ::fopen(filename.c_str(), "w");
   }
};

#endif
