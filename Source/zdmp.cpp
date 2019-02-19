//------------------------------------------------------------------------------
// Copyright (c) 2018 John D. Haughton
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

#include "share/Quetzal.h"
#include "Z/Story.h"

#include "STB/ConsoleApp.h"

#include <fstream>
#include <iostream>
#include <iomanip>

#define  PROGRAM         "ZDmp"
#define  DESCRIPTION     "Dump Z story contents"
#define  LINK            "https://github.com/AnotherJohnH/Zif"
#define  AUTHOR          "John D. Haughton"
#define  VERSION         PROJ_VERSION
#define  COPYRIGHT_YEAR  "2018-2019"

//!
class ZDmp : public STB::ConsoleApp
{
private:
   STB::Option<bool>        dump_mem{'d', "mem", "Dump memory", false};
   STB::Option<const char*> save_file{'s', "save", "Save file"};
   STB::Option<const char*> output_file{'o', "out", "Output file"};

   std::ofstream out_file_stream;
   std::ostream* out{&std::cout};

   std::string filename;
   Z::Story    story;
   IF::Quetzal quetzal;
   IF::State   state{story, 0, 2048};

   int error(const std::string& message)
   {
      std::cerr << "ERR - " << message << std::endl;
      return -1;
   }

   void attr(const std::string& name, const std::string& value)
   {
      *out << "  \"" << name << "\": \"" << value << "\"," << std::endl;
   }

   void attr(const std::string& name, unsigned value)
   {
      *out << std::hex << std::setfill('0');
      *out << "  \"" << name << "\": \"0x" << value << "\"," << std::endl;
   }

   void dumpHeader(const Z::Header* header)
   {
      attr("version",          header->version);
      attr("flags1",           header->flags1);
      attr("release",          header->release);
      attr("himem",            header->himem);
      attr("initPC",           header->init_pc);
      attr("dict",             header->dict);
      attr("obj",              header->obj);
      attr("globalBase",       header->glob);
      attr("static",           header->stat);
      attr("gameEnd",          header->getStorySize());
      attr("memLimit",         header->getMemoryLimit());
      attr("flags2",           header->flags2);
      attr("abbr",             header->abbr);
      attr("length",           header->length);
      attr("checksum",         header->checksum);
      attr("interpNum",        header->interpreter_number);
      attr("interpVer",        header->interpreter_version);
      attr("screenLines",      header->screen_lines);
      attr("screenCols",       header->screen_cols);
      attr("screenWidth",      header->screen_width);
      attr("screenHeight",     header->screen_height);
//      attr("fontWidth",       header->font_width);
//      attr("fontHeight",      header->font_height);
      attr("routines",         header->routines);
      attr("staticStrings",    header->static_strings);
      attr("bgColour",         header->background_colour);
      attr("fgColour",         header->foreground_colour);
      attr("termChars",        header->terminating_characters);
      attr("widthTextStream3", header->width_text_stream3);
      attr("standardRevision", header->standard_revision);
      attr("alphabetTable",    header->alphabet_table);
      attr("headerExt",        header->header_ext);
   }

   void dumpMemory()
   {
      *out << "  \"memory\": [" << std::endl;

      for(unsigned addr=0; addr<state.memory.size(); addr += 16)
      {
         *out << "    {\"a\": \"" << std::setw(6) << addr << "\", \"b\": \"";

         for(unsigned i=0; i<16; i++)
         {
            if ((addr + i) < state.memory.size())
            {
               *out << " " << std::setw(2) << unsigned(state.memory.read8(addr + i));
            }
         }

         *out << "\", \"c\": \"";

         for(unsigned i=0; i<16; i++)
         {
            if ((addr + i) < state.memory.size())
            {
               uint8_t ch = state.memory.read8(addr + i);
               if (isprint(ch))
               {
                  *out << ch;
               }
               else
               {
                  *out << '.';
               }
            }
         }

         *out << "\"}," << std::endl;
      }

      *out << "  ]," << std::endl;
   }

   void dumpStack()
   {
      *out << "  \"stack\": [" << std::endl;

      for(unsigned i=0; i<state.stack.size(); i++)
      {
         *out << "    \"0x" << std::setw(4) << state.stack.data()[i] << "\"," << std::endl;
      }

      *out << "  ]" << std::endl;
   }

   virtual int startConsoleApp() override
   {
      if (output_file != nullptr)
      {
         out_file_stream.open(output_file, std::ofstream::binary);
         if (out_file_stream.is_open())
         {
            out = &out_file_stream;
         }
      }

      *out << "{" << std::endl;

      attr("story", filename);
      if (!story.load(filename))
      {
         return error(story.getLastError());
      }

      story.prepareMemory(state.memory);

      if (save_file != nullptr)
      {
         attr("saveFile", (const char*)save_file);
         if (!quetzal.read((const char*)save_file))
         {
            return error(quetzal.getLastError());
         }

         if (!quetzal.decode(story, state))
         {
            return error(quetzal.getLastError());
         }

         attr("PC", state.getPC());
         attr("randState", state.random.internalState());
      }
      else
      {
         story.resetMemory(state.memory);
      }

      dumpHeader((const Z::Header*)state.memory.data());

      if (dump_mem)
      {
         dumpMemory();
      }

      if (save_file != nullptr)
      {
         dumpStack();
      }

      *out << "}" << std::endl;

      return 0;
   }

   virtual void parseArg(const char* arg) override
   {
      filename = arg;
   }

public:
   ZDmp(int argc, const char* argv[])
      : ConsoleApp(PROGRAM, DESCRIPTION, LINK, AUTHOR, VERSION, COPYRIGHT_YEAR)
   {
      parseArgsAndStart(argc, argv);
   }
};


int main(int argc, const char* argv[])
{
   ZDmp(argc, argv);
}
