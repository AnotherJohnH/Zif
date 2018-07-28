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

#ifndef ERROR_H
#define ERROR_H


enum class Error
{
   NONE = 0,

   UNIMPLEMENTED_OP,
   ILLEGAL_OP,
   STACK_EMPTY,
   STACK_UNDERFLOW,
   STACK_OVERFLOW,
   BAD_FRAME_PTR,
   BAD_PC,
   BAD_ADDRESS,
   BAD_STREAM,
   BAD_CALL_TYPE,
   DIV_BY_ZERO,
   BAD_CONFIG,
   BAD_VERSION
};

//! Check code for an error
inline bool isError(Error code)
{
   return code != Error::NONE;
}

//! Convert error code into a string
inline const char* errorString(Error code)
{
   // clang-format off
   switch(code)
   {
   case Error::NONE:             return "no error";
   case Error::UNIMPLEMENTED_OP: return "Unimplement op";
   case Error::ILLEGAL_OP:       return "Illegal op";
   case Error::STACK_EMPTY:      return "Stack empty";
   case Error::STACK_UNDERFLOW:  return "Stack underflow";
   case Error::STACK_OVERFLOW:   return "Stack overflow";
   case Error::BAD_FRAME_PTR:    return "Bad frame pointer";
   case Error::BAD_PC:           return "Bad PC";
   case Error::BAD_ADDRESS:      return "Bad address";
   case Error::BAD_STREAM:       return "Bad stream";
   case Error::BAD_CALL_TYPE:    return "Bad call type";
   case Error::DIV_BY_ZERO:      return "Division by zero";
   case Error::BAD_CONFIG:       return "Bad config";
   case Error::BAD_VERSION:      return "Bad Z version";
   }
   // clang-format on

   return "UNRECOGNISED ERROR CODE";
}


#endif
