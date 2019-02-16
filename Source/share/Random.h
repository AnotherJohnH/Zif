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

#ifndef IF_RANDOM_H
#define IF_RANDOM_H

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
   void seed(uint32_t seed_)
   {
      state = seed_ == 0 ? 1 : seed_;
   }

   //! Set an unpredictable seed
   void unpredictableSeed()
   {
      // TODO seed with an unpredictable value
      seed(1);
   }

   //! Get the next pseudo random value
   uint32_t get()
   {
      // use basic xorshift, it's fast and simple
      // TODO change to the 64-bit version for improved
      //      statistical characteristics for similar cost
      state ^= uint32_t(state << 13);
      state ^= uint32_t(state >> 17);
      state ^= uint32_t(state << 5);
      return state;
   }

private:
   uint64_t state{1};
};

} // namespace IF

#endif
