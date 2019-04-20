
#include <cstdarg>
#include <cstdlib>
#include <cstdio>

#include <thread>

#include <termios.h>
#include <sys/select.h>

#include <string>


static termios* getTio()
{
   static termios tio;
   return &tio;
}

static void restoreTio()
{
   tcsetattr(0, TCSANOW, getTio());
   ::puts("\e[?25h");
}

static void saveTio()
{
   tcgetattr(0, getTio());

   atexit(restoreTio);
}


static void modifyTioFlag(unsigned flag, bool set)
{
   struct termios tio;

   tcgetattr(0, &tio);

   if(set)
      tio.c_lflag |= flag;
   else
      tio.c_lflag &= ~flag;

   tcsetattr(0, TCSANOW, &tio);
}


void resp(FILE* pp)
{
   while(true)
   {
      uint8_t ch = getchar();
      int status = fputc(ch, pp);
      if (status < 0) break;
   }
}


int main()
{
   saveTio();

   modifyTioFlag(ICANON, 1);
   modifyTioFlag(ECHO,   1);

   std::string ext_cmd;

   ext_cmd = "vi";
   ext_cmd += " 2>&1";

   FILE* pp = popen(ext_cmd.c_str(), "r+");
   if (pp == nullptr)
   {
      fprintf(stderr, "Failed to open pipe\n");
      exit(1);
   }

   std::thread io(resp, pp);

   while(true)
   {
      int ch = fgetc(pp);
      if (ch < 0) break;
      //printf("[%02X]", ch);
      putchar(ch);
   }

   io.join();

   pclose(pp);
}

