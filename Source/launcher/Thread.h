//------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
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

#ifndef THREAD_H
#define THREAD_H

#include <atomic>

#include <pthread.h>

//! A poor replacement for std::thread
//  that may be more likely to work on older OSes
class Thread
{
public:
   Thread() = default;

   virtual ~Thread()
   {
      if (!joined)
      {
         if (pthread_cancel(td) != 0)
         {
            perror("pthread_cancel");
         }
      }
   }

   void join()
   {
      pthread_join(td, nullptr);
      joined = true;
   }

protected:
   void start()
   {
      pthread_attr_t attr;
      if (pthread_attr_init(&attr) != 0)
      {
         perror("pthread_attr_init");
         exit(1);
      }

      if (pthread_create(&td, &attr, thunk, this) != 0)
      {
         perror("pthread_create");
         exit(1);
      }
   }

   virtual void entry() = 0;

private:
   static void* thunk(void* ptr)
   {
      Thread* that = (Thread*)ptr;
      that->entry();
      return nullptr;
   }

   pthread_t         td{};
   std::atomic<bool> joined{false};
};

#endif
