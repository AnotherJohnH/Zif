//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstring>
#include <cstdint>

#include "common/Memory.h"

namespace Z {

//! Translator of input commands into tokens
class Parser
{
private:
   uint8_t version;

   //! Encoded word
   class ZWord
   {
   private:
      static const unsigned MAX_ZWORD = 3;

      uint8_t  max_len;
      uint8_t  len{0};
      uint16_t word[MAX_ZWORD];

   public:
      ZWord(uint8_t version)
         : max_len(version < 4 ? 6 : 9)
      {
         for(unsigned i = 0; i < size(); i++)
         {
            word[i] = 0;
         }
      }

      //! Return number of 16-bit words
      unsigned size() const { return max_len / 3; }

      //! Read a 16-bit word
      uint16_t operator[](unsigned index) { return word[index]; }

      //! Encode the next character
      void push(uint8_t zch)
      {
         if(len >= max_len) return;

         unsigned index = len / 3;
         word[index]    = (word[index] << 5) | zch;

         len++;
      }

      //! Terminate the word
      void terminate()
      {
         while(len < max_len)
         {
            push(5);
         }

         word[size() - 1] |= 1 << 15;
      }
   };

   void encode(const uint8_t* word, ZWord& zword)
   {
      // Alphabet table (v1) [3.5.4]
      static const char* alphabet_v1 = "abcdefghijklmnopqrstuvwxyz"    // A0
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"    // A1
                                       " 0123456789.,!?_#'\"/\\<-:()"; // A2

      // Alphabet table (v2 - v4) [3.5.3]
      static const char* alphabet_v2_v4 = "abcdefghijklmnopqrstuvwxyz"     // A0
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"     // A1
                                          " \n0123456789.,!?_#'\"/\\-:()"; // A2

      const char* alphabet = version == 1 ? alphabet_v1 : alphabet_v2_v4;

      for(unsigned i = 0; i < word[i]; ++i)
      {
         uint8_t ch = word[i];

         const char* s = strchr(alphabet, ch);
         if(s)
         {
            uint8_t zch = (s - alphabet) % 26;
            uint8_t za  = (s - alphabet) / 26;

            if((version == 1) || (version == 2))
            {
                    if(za == 1) zword.push(1);
               else if(za == 2) zword.push(2);
            }
            else
            {
                    if(za == 1) zword.push(4);
               else if(za == 2) zword.push(5);
            }

            zword.push(zch + 6);
         }
      }

      zword.terminate();
   }

   int compare(IF::Memory& memory, uint32_t entry_addr, ZWord& zword)
   {
      for(unsigned i = 0; i < zword.size(); i++)
      {
         if(memory.read16(entry_addr + i * 2) != zword[i]) return -1;
      }

      return 0;
   }

public:
   Parser(unsigned version_)
      : version(version_)
   {
   }

   //! Translate input command into list of tokens in memory
   void tokenise(IF::Memory& memory, uint32_t out, uint32_t in, uint32_t dict, bool partial)
   {
      uint8_t  max_word_len = version <= 4 ? 6 : 9;
      uint8_t  num_sep      = memory.fetch8(dict);
      uint8_t  entry_length = memory.fetch8(dict + 1 + num_sep);
      uint16_t num_entry    = memory.fetch16(dict + 1 + num_sep + 1);
      uint32_t first        = dict + 1 + num_sep + 3;

      unsigned max_num_word = memory.read8(out);
      uint8_t  num_word     = 0;

      uint8_t  word_len = 0;
      uint8_t  word[9 + 1];
      uint8_t  start = 0;

      for(unsigned i = 0; i < 256; i++)
      {
         uint8_t ch = memory.read8(in + i);

         // Check for default seperator and terminator
         bool is_sep = (ch == ' ') || (ch == '\0');

         // Check for custom seperators
         for(uint8_t j = 0; !is_sep && (j < num_sep); j++)
         {
            is_sep = (memory.fetch8(dict + 1 + j) == ch);
         }

         if(!is_sep)
         {
            if(word_len == 0)
            {
               start = i;
            }
            if(word_len < max_word_len)
            {
               word[word_len++] = ch;
            }
         }
         else if(word_len > 0)
         {
            word[word_len] = '\0';

            ZWord zword(version);

            encode(word, zword);

            uint16_t entry = 0;

            for(uint16_t j = 0; j < num_entry; j++)
            {
               entry = first + j * entry_length;

               if(compare(memory, entry, zword) == 0)
               {
                  break;
               }
               else if((j == num_entry - 1) && !partial)
               {
                  entry = 0;
               }
            }

            if((entry != 0) || !partial)
            {
               memory.write16(out + 2 + num_word * 4, entry);
               memory.write8(out + 2 + num_word * 4 + 2, word_len);
               memory.write8(out + 2 + num_word * 4 + 3, start + 2);
            }

            num_word++;

            word_len = 0;
         }

         if((ch == '\0') || (num_word == max_num_word)) break;
      }

      memory.write8(out + 1, num_word);
   }
};

} // namespace Z

