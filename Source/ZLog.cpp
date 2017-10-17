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

#include <cassert>
#include <cstdarg>
#include <cstdio>

#include "ZLog.h"


ZLog::~ZLog()
{
   if(handle != nullptr)
   {
      fclose((FILE*)handle);
   }
}


void ZLog::ensureOpen()
{
   if(handle == nullptr)
   {
      char filename[FILENAME_MAX];
      sprintf(filename, "%s.log", name);
      handle = fopen(filename, "w");
      assert(handle); // TODO report an error to the user
   }
}


void ZLog::write(char ch)
{
   ensureOpen();

   fputc(ch, (FILE*)handle);
}


void ZLog::printf(const char* format, ...)
{
   ensureOpen();

   FILE* fp = (FILE*)handle;

   va_list ap;
   va_start(ap, format);
   vfprintf(fp, format, ap);
   va_end(ap);

   fflush(fp);
}
