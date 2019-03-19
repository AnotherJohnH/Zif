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

#ifndef TO_HEX_STRING_H
#define TO_HEX_STRING_H

// Not entirely sure why I needed to write this!
template <typename TYPE>
static void addHexString(std::string& text, TYPE value, unsigned digits = 0, char pad = '0')
{  
   for(signed n = sizeof(TYPE) * 2 - 1; n >= 0; n--)
   {  
      unsigned digit = (value >> (n * 4)) & 0xF;
      
      if ((n != 0) && (digit == 0))
      {  
         if ((value >> (n * 4)) != 0)
         {  
            text += '0';
         }
         else if (n <= signed(digits))
         {  
            text += pad;
         }
      }
      else
      {  
         text += digit > 9 ? 'A' + digit - 10
                           : '0' + digit;
      }
   }
}

#endif
