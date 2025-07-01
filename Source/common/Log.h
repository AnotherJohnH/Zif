//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdio>
#include <string>

//! A log file
class Log
{
public:
   Log(const std::string& filename_)
      : filename(filename_)
   {
   }

   ~Log()
   {
      if (fp != nullptr) ::fclose(fp);
   }

   //! write a single character to the log
   void write(char ch)
   {
      ensureOpen();

      if (suffix != "")
      {
         // partial write in progress => end it
         for(const auto& ch : suffix)
         {
            ::fputc(ch, fp);
         }

         prefix = "";
         suffix = "";
      }

      ::fputc(ch, fp);
   }

   //! write a string to the log
   void write(const std::string& str)
   {
      for(const auto& ch : str)
      {
         write(ch);
      }
   }

   //! Start a partial write with prefix and suffix strings
   void writePart(const std::string& prefix_, char ch, const std::string& suffix_)
   {
      ensureOpen();

      if (prefix != prefix_)
      {
         // start a new partial write
         write(prefix_);
         prefix = prefix_;
         suffix = suffix_;
      }

      ::fputc(ch, fp);
   }

private:
   std::string filename;
   FILE*       fp{nullptr};
   std::string prefix{};
   std::string suffix{};

   void ensureOpen()
   {
      if(fp == nullptr) fp = ::fopen(filename.c_str(), "w");
   }
};

