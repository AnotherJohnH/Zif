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

#ifndef ZLOG_H
#define ZLOG_H

#include <cstdarg>

#include "PLT/File.h"

//! Log file
class ZLog : public PLT::File
{
public:
   ZLog(const char* name_)
      : PLT::File(nullptr, name_, "log")
   {
   }

   void write(char ch)
   {
      ensureOpen();

      PLT::File::write(&ch, 1);
   }

   void printf(const char* format, ...)
   {
      ensureOpen();

      va_list ap;
      va_start(ap, format);
      PLT::File::vprintf(format, ap);
      va_end(ap);

      PLT::File::flush();
   }

private:
   void ensureOpen()
   {
      if (!isOpen()) openForWrite();
   }
};

#endif
