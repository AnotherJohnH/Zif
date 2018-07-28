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


enum Error
{
   NO_ERROR = 0,

   ERR_UNIMPLEMENTED_OP,
   ERR_ILLEGAL_OP,
   ERR_STACK_EMPTY,
   ERR_STACK_UNDERFLOW,
   ERR_STACK_OVERFLOW,
   ERR_BAD_FRAME_PTR,
   ERR_BAD_PC,
   ERR_BAD_ADDRESS,
   ERR_BAD_STREAM,
   ERR_BAD_CALL_TYPE,
   ERR_DIV_BY_ZERO,
   ERR_BAD_CONFIG,
   ERR_BAD_VERSION
};


inline const char* errorString(Error code)
{
   switch(code)
   {
   case ERR_UNIMPLEMENTED_OP: return "Unimplement op";
   case ERR_ILLEGAL_OP:       return "Illegal op";
   case ERR_STACK_EMPTY:      return "Stack empty";
   case ERR_STACK_UNDERFLOW:  return "Stack underflow";
   case ERR_STACK_OVERFLOW:   return "Stack overflow";
   case ERR_BAD_FRAME_PTR:    return "Bad frame pointer";
   case ERR_BAD_PC:           return "Bad PC";
   case ERR_BAD_ADDRESS:      return "Bad address";
   case ERR_BAD_STREAM:       return "Bad stream";
   case ERR_BAD_CALL_TYPE:    return "Bad call type";
   case ERR_DIV_BY_ZERO:      return "Division by zero";
   case ERR_BAD_CONFIG:       return "Bad config";
   case ERR_BAD_VERSION:      return "Bad Z version";

   default: break;
   }

   return nullptr;
}


#endif
