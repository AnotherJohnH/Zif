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

#include <array>
#include <sys/stat.h>
#include <unistd.h>

#include "STB/ConsoleApp.h"
#include "STB/Option.h"
#include "STB/XML.h"
#include "STB/Http.h"

#define  PROGRAM         "zscrape"
#define  DESCRIPTION     "Zcrape IF archive using http for story files (please be nice!)"
#define  LINK            "https://github.com/AnotherJohnH/"
#define  COPYRIGHT_YEAR  "2018-2019"
#define  AUTHOR          "John D. Haughton"
#define  VERSION         PROJ_VERSION

class ZcrapeApp : public STB::ConsoleApp
{
private:
   STB::Option<const char*> host{   'H', "host",  "Remote hostname",   "ifarchive.org"};
   STB::Option<const char*> path{   'P', "path",  "Remote path",       "/if-archive/games/zcode/"};
   STB::Option<const char*> cache{  'C', "cache", "Download directory","Games/Downloads"};
   STB::Option<unsigned>    delay{  'D', "delay", "Delay per GET (s)", 1};

   //! Download file
   bool getFile(STB::Http&         http,
                const std::string& path_,
                const std::string& file_)
   {
      printf("GET http://%s%s as \"%s\"", (const char*)host, path_.c_str(), file_.c_str());
      bool ok = http.getFile(path_, file_);
      printf(" - %s\n", ok ? "OK" : "FAIL");

      // XXX sleep here for a bit to avoid anoying the server
      //     and other users of the server
      sleep(delay);

      return ok;
   }

   //! Scan HTML for hyperlinks to story files
   void findLinks(STB::Http& http, const STB::XML::Element& xml)
   {
      const char* ext_list[] =
      {
         ".z1", ".z2", ".z3", ".z4", ".z5", ".z6", ".z7", ".z8",
         ".Z1", ".Z2", ".Z3", ".Z4", ".Z5", ".Z6", ".Z7", ".Z8",
         ".zip", ".zblorb", nullptr
      };

      for(const auto& element : xml)
      {
         if (element.getName() == "a")
         {
            std::string href = element["href"];

            // TODO handle hrefs that include a path

            for(unsigned i=0; ext_list[i] != nullptr; i++)
            {
               if (href.find(ext_list[i]) != std::string::npos)
               {
                  std::string local_path = (const char*)path;
                  local_path += href;

                  std::string file = (const char*) cache;
                  file += '/';
                  file += href;

                  if (PLT::File::size(file.c_str()) == 0)
                  {
                      getFile(http, local_path, file);
                  }
                  break;
               }
            }
         }

         findLinks(http, element);
      }
   }

   virtual int startConsoleApp() override
   {
      mkdir(cache, 0777);

      STB::Http http;

      if(http.open((const char*)host))
      {
         std::string file = (const char*) cache;
         file += "/index.html";

         if (getFile(http, (const char*)path, file))
         {
            STB::XML::Document xml(file, /* require_prolog */ false);

            findLinks(http, xml);
         }

         http.close();
      }

      return 0;
   }

public:
   ZcrapeApp(int argc, const char* argv[])
      : ConsoleApp(PROGRAM, DESCRIPTION, LINK, AUTHOR, VERSION, COPYRIGHT_YEAR)
   {
      parseArgsAndStart(argc, argv);
   }
};


int main(int argc, const char* argv[])
{
   ZcrapeApp(argc, argv);
}
