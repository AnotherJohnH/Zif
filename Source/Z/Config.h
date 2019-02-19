//------------------------------------------------------------------------------
// Copyright (c) 2016-2017 John D. Haughton
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

#ifndef Z_CONFIG_H
#define Z_CONFIG_H

#include <cstdint>

namespace Z {

//! Configuration of interpreter features
struct Config
{
   uint8_t  interp_major_version{0};
   uint8_t  interp_minor_version{0};
   bool     status_line{true};       //!< Status line available
   bool     screen_splitting{false}; //!< Screen splitting available
   bool     var_pitch_font{false};   //!< variable pitch font the default
   bool     pictures{false};         //!< picture display available
   bool     sounds{false};           //!< sounds available
   bool     transcripting{false};    //!< transcripting starts on
};

} // namespace Z

#endif
