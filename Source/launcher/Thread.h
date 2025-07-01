//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

