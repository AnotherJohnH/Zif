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

#include "Z/ZMemory.h"
#include "Z/ZQuetzal.h"
#include "Z/ZStack.h"
#include "Z/ZStory.h"

#include "STB/ConsoleApp.h"

#include <iostream>
#include <iomanip>

#define  PROGRAM         "QDmp"
#define  DESCRIPTION     "Dump Z story contents"
#define  LINK            "https://github.com/AnotherJohnH/Zif"
#define  AUTHOR          "John D. Haughton"
#define  VERSION         PROJ_VERSION
#define  COPYRIGHT_YEAR  "2018"

//!
class QDmp : public STB::ConsoleApp
{
private:
   STB::Option<const char*> save_file{'s', "save", "Save file"};

   std::string filename;
   ZStory      story;
   ZQuetzal    quetzal;

   uint32_t    pc;
   uint32_t    rand_state;
   ZMemory     memory;
   ZStack      stack;

   int error(const std::string& message)
   {
      std::cerr << "ERR - " << message << std::endl;
      return -1;
   }

   void attr(const std::string& name, const std::string& value)
   {
      std::cout << "  \"" << name << "\": \"" << value << "\"," << std::endl;
   }

   void attr(const std::string& name, unsigned value)
   {
      std::cout << std::hex << std::setfill('0');
      std::cout << "  \"" << name << "\": \"0x" << value << "\"," << std::endl;
   }

   void dumpHeader(const ZHeader* header)
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
      std::cout << "  \"memory\": [" << std::endl;

      for(unsigned addr=0; addr<memory.getSize(); addr += 16)
      {
         std::cout << "    {\"a\": \"" << std::setw(6) << addr << "\", \"b\": \"";

         for(unsigned i=0; i<16; i++)
         {
            if ((addr + i) < memory.getSize())
            {
               std::cout << " " << std::setw(2) << unsigned(memory.get(addr + i));
            }
         }

         std::cout << "\", \"c\": \"";

         for(unsigned i=0; i<16; i++)
         {
            if ((addr + i) < memory.getSize())
            {
               uint8_t ch = memory.get(addr + i);
               if (isprint(ch))
               {
                  std::cout << ch;
               }
               else
               {
                  std::cout << '.';
               }
            }
         }

         std::cout << "\"}," << std::endl;
      }

      std::cout << "  ]," << std::endl;
   }

   void dumpStack()
   {
      std::cout << "  \"stack\": [" << std::endl;

      for(unsigned i=0; i<stack.size(); i++)
      {
         std::cout << "    \"0x" << std::setw(4) << stack[i] << "\"," << std::endl;
      }

      std::cout << "  ]" << std::endl;
   }

   virtual int startConsoleApp() override
   {
      std::cout << "{" << std::endl;

      attr("story", filename);
      if (!story.load(filename))
      {
         return error(story.getLastError());
      }

      const ZHeader* header = story.getHeader();
      memory.configure(header);

      if (save_file != nullptr)
      {
         attr("saveFile", (const char*)save_file);
         if (!quetzal.read((const char*)save_file))
         {
            return error(quetzal.getLastError());
         }

         if (!quetzal.decode(story, pc, rand_state, memory, stack))
         {
            return error(quetzal.getLastError());
         }

         attr("PC", pc);
         attr("randState", rand_state);

         header = memory.getHeader();
      }
      else
      {
         memcpy(memory.getData(), story.data(), story.size());
      }

      dumpHeader(header);
      dumpMemory();

      if (save_file != nullptr)
      {
         dumpStack();
      }

      std::cout << "}" << std::endl;

      return 0;
   }

   virtual void parseArg(const char* arg) override
   {
      filename = arg;
   }

public:
   QDmp(int argc, const char* argv[])
      : ConsoleApp(PROGRAM, DESCRIPTION, LINK, AUTHOR, VERSION, COPYRIGHT_YEAR)
   {
      parseArgsAndStart(argc, argv);
   }
};


int main(int argc, const char* argv[])
{
   QDmp(argc, argv);
}
