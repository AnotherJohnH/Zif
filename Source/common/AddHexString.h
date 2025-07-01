//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

