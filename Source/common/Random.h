//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace IF {

//! Pseudo random number generator for interactive fiction VM
class Random
{
public:
   Random()
   {
      unpredictableSeed();
   }

   //! Access to internal state for save/restore
   uint64_t& internalState() { return state; }

   //! Access to internal state for save/restore
   const uint64_t& internalState() const { return state; }

   //! Seed the random number generator
   void predictableSeed(uint32_t seed_)
   {
      state = seed_ == 0 ? 1 : seed_;
   }

   //! Set an unpredictable seed
   void unpredictableSeed()
   {
      // TODO seed with an unpredictable value
      predictableSeed(1);
   }

   //! Set a the generator into sequential mode
   void sequentialSeed(uint32_t limit_)
   {
      state            = 0;
      sequential_limit = limit_;
   }

   //! Get the next pseudo random value
   uint32_t get()
   {
      if (sequential_limit != 0)
      {
          state++;
          if (state == sequential_limit)
          {
             state = 0;
          }
      }
      else
      {
          // use basic xorshift, it's fast and simple
          state ^= state << 13;
          state ^= state >> 7;
          state ^= state << 17;
      }
      return state;
   }

private:
   uint64_t state{1};
   uint32_t sequential_limit{0};
};

} // namespace IF

