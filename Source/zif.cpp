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

#include "ConsoleImpl.h"
#include "Options.h"
#include "Z/ZMachine.h"
#include "Glulx/Machine.h"
#include "share/Blorb.h"

#include "TRM/Launcher.h"

#define  PROGRAM         "Zif"
#define  DESCRIPTION     "Z-code engine for interactive fiction"
#define  LINK            "https://github.com/AnotherJohnH/Zif"
#define  AUTHOR          "John D. Haughton"
#define  VERSION         PROJ_VERSION
#define  COPYRIGHT_YEAR  "2015-2019"


//! The Zif Launcher Application
class ZifApp : public TRM::Launcher
{
private:
   Options options;

   virtual int startTerminalLauncher(const char* story_file) override
   {
      ConsoleImpl console(term, options);

      Blorb       blorb;
      std::string exec_type;
      uint32_t    exec_offset{0};

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
            ZMachine machine(console, options, z_story);
            return machine.play() ? 0 : 1;
         }
         else
         {
            console.error(z_story.getLastError());
            return 1;
         }
      }

      Glulx::Story glulx_story;
      if ((exec_type == "GLUL") || glulx_story.isRecognised(story_file))
      {
         if (glulx_story.load(story_file, exec_offset))
         {
            Glulx::Machine machine(console, options, glulx_story);
            return machine.play() ? 0 : 1;
         }
         else
         {
            console.error(glulx_story.getLastError());
            return 1;
         }
      }

      console.error("Story file format not recognised");
      return 1;
   }

public:
   ZifApp(int argc, const char* argv[])
      : TRM::Launcher(PROGRAM, DESCRIPTION, LINK, AUTHOR, VERSION, COPYRIGHT_YEAR,
                      "[<story-file>]", "zif.cfg")
   {
      parseArgsAndStart(argc, argv);
   }
};


int main(int argc, const char* argv[])
{
   ZifApp(argc, argv);
}
