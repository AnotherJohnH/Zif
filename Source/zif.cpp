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

#include "share/ConsoleImpl.h"
#include "share/Options.h"
#include "share/Blorb.h"

#include "Z/Machine.h"
#include "Glulx/Machine.h"
#include "Level9/Machine.h"

#include "launcher/Launcher.h"

#define  PROGRAM         "Zif"
#define  DESCRIPTION     "Z-code engine for interactive fiction"
#define  LINK            "https://github.com/AnotherJohnH/Zif"
#define  AUTHOR          "John D. Haughton"
#define  VERSION         PROJ_VERSION
#define  COPYRIGHT_YEAR  "2015-2019"


//! The Zif Launcher Application
class ZifApp : public Launcher
{
private:
   Options options;

   int error(Console& console, const std::string& message)
   {
      console.error(message);
      console.waitForKey();
      return 1;
   }

   virtual bool hasSaveFile(const std::string& story_file) const override
   {
      size_t slash = story_file.rfind('/');

      std::string save_path = (const char*)options.save_dir;
      save_path += '/';
      save_path += story_file.c_str() + slash;
      save_path += ".qzl";

      FILE* fp = fopen(save_path.c_str(), "rb");
      if (fp != nullptr)
      {
         fclose(fp);
         return true;
      }

      return false;
   }

   virtual int runGame(const char* story_file, bool restore) override
   {
      ConsoleImpl console(term, options);
      Blorb       blorb;
      std::string exec_type;
      uint32_t    exec_offset{0};

      FILE* fp = fopen(story_file, "r");
      if (fp == nullptr)
      {
         return error(console,"Story file could not be opened");
      }
      fclose(fp);

      if (!blorb.findResource(story_file,
                              Blorb::Resource::EXEC,
                              /* index */ 0,
                              exec_type,
                              exec_offset))
      {
         exec_type   = "?";
         exec_offset = 0;
      }

      Z::Story z_story;
      if ((exec_type == "ZCOD") || z_story.isRecognised(story_file))
      {
         if (z_story.load(story_file, exec_offset))
         {
            Z::Machine machine(console, options, z_story);
            return machine.play(restore) ? 0 : 1;
         }
         else
         {
            return error(console, z_story.getLastError());
         }
      }

      Glulx::Story glulx_story;
      if ((exec_type == "GLUL") || glulx_story.isRecognised(story_file))
      {
         if (glulx_story.load(story_file, exec_offset))
         {
            Glulx::Machine machine(console, options, glulx_story);
            return machine.play(restore) ? 0 : 1;
         }
         else
         {
            return error(console, glulx_story.getLastError());
         }
      }

      Level9::Story level9_story;
      if ((exec_type == "LEVE") || level9_story.isRecognised(story_file))
      {
         if (level9_story.load(story_file, exec_offset))
         {
            Level9::Machine machine(console, options, level9_story);
            return machine.play(restore) ? 0 : 1;
         }
         else
         {
            return error(console, level9_story.getLastError());
         }
      }

      return error(console,"Story file format not recognised");
   }

public:
   ZifApp(int argc, const char* argv[])
      : Launcher(PROGRAM, DESCRIPTION, LINK, AUTHOR, VERSION, COPYRIGHT_YEAR,
                 "[<story-file>]")
   {
      parseArgsAndStart(argc, argv);
   }
};


int main(int argc, const char* argv[])
{
   ZifApp(argc, argv);
}
