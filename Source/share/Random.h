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

#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>

//! Pseudo random number generator
class Random
{
public:
   Random()
   {
      unpredictableSeed();
   }

   //! Access to internal state
   uint32_t& internalState() { return state; }

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

   //! Get the next random value
   uint32_t get()
   {
      // use xorshift, it's fast and simple
      state ^= state << 13;
      state ^= state >> 17;
      state ^= state << 5;
      return state;
   }

private:
   uint32_t state{1};
};

#endif
