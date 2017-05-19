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

#include  "STB/ConsoleApp.h"

#include  "PLT/TerminalPaper.h"
#include  "PLT/TerminalStdio.h"

#include  "ZifVersion.h"
#include  "ZLauncher.h"


class Zif : public STB::ConsoleApp
{
private:
   enum Display
   {
      DISP_KINDLE3,
      DISP_VGA,
      DISP_SVGA,
      DISP_XGA
   };

#ifdef PROJ_TARGET_Kindle3
   Display display = DISP_KINDLE3;
#else
   Display display = DISP_SVGA;
#endif

   STB::Option<const char*>  opt_config{ 'c', "config", "Config file", "zif.cfg"};
   STB::Option<bool>         opt_term{   't', "term",   "Use the parent terminal"};
   STB::Option<bool>         opt_k3{     'K', "k3",     "Kindle display 800x600"};
   STB::Option<bool>         opt_vga{    'V', "vga",    "VGA display    640x480"};
   STB::Option<bool>         opt_svga{   'S', "svga",   "SVGA display   800x600"};
   STB::Option<bool>         opt_xga{    'X', "xga",    "XGA display   1024x768"};
   const char*               filename{nullptr};

   int launch(PLT::Device& term)
   {
      ZLauncher  launcher(term, opt_config);

      return filename ? launcher.run(filename)
                      : launcher.menu();
   }

   template <unsigned WIDTH, unsigned HEIGHT>
   int launchDisplay()
   {
      PLT::TerminalPaper<WIDTH,HEIGHT>  term(PROGRAM);
      return launch(term);
   }

   virtual int start() override
   {
      if (argc == 2)
      {
          filename = argv[1];
      }

      if (opt_term)
      {
         // Use the parent terminal
         PLT::TerminalStdio  term(PROGRAM);
         return launch(term);
      }
      else
      {
         // Use the built in terminal

              if (opt_k3)   display = DISP_KINDLE3;
         else if (opt_vga)  display = DISP_VGA;
         else if (opt_svga) display = DISP_SVGA;
         else if (opt_xga)  display = DISP_XGA;

         switch(display)
         {
         case DISP_KINDLE3: return launchDisplay< 600,800>();
         case DISP_VGA:     return launchDisplay< 640,480>();
         case DISP_SVGA:    return launchDisplay< 800,600>();
         case DISP_XGA:     return launchDisplay<1024,768>();
         }
      }
   }

public:
   Zif(int argc_, const char* argv_[])
      : ConsoleApp(argc_, argv_,
                   PROGRAM, AUTHOR, DESCRIPTION, VERSION, COPYRIGHT_YEAR, LICENSE,
                   "[Z-file]")
   {
      parseArgsAndStart();
   }
};


int main(int argc, const char *argv[])
{
   Zif(argc, argv);
}

