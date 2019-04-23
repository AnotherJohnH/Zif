//------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
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

#ifndef PIPE_H
#define PIPE_H

#include <cstdio>
#include <cstdlib>

#include <unistd.h>

class Pipe
{
public:
   Pipe()
   {
      int status = ::pipe(fds);
      if (status < 0)
      {
         perror("pipe");
         exit(1);
      }

      closed[0] = false;
      closed[1] = false;
   }

   ~Pipe()
   {
      closeEnd(READ);
      closeEnd(WRITE);
   }

   //! Close read end of pipe and dup write file descriptor
   void assignWriteFD(int new_fd)
   {
      closeEnd(READ);
      if (::dup2(fds[WRITE], new_fd) == -1)
      {
         perror("dup2");
         exit(1);
      }
      closeEnd(WRITE);
   }

   //! Close write end of pipe and dup read file descriptor
   void assignReadFD(int new_fd)
   {
      closeEnd(WRITE);
      if (::dup2(fds[READ], new_fd) == -1)
      {
         perror("dup2");
         exit(1);
      }
      closeEnd(READ);
   }

   //! Close write end of pipe and return file descriptor for read end
   int getReadFD()
   {
      closeEnd(WRITE);
      return fds[READ];
   }

   //! Close read end of pipe and return file descriptor for write end
   int getWriteFD()
   {
      closeEnd(READ);
      return fds[WRITE];
   }

private:
   enum End : unsigned
   {
       READ  = 0,
       WRITE = 1
   };
  
   int  fds[2];
   bool closed[2];

   //! Close an one end of the pipe if not closed already
   void closeEnd(End index)
   {
      if (!closed[index])
      {
         ::close(fds[index]);
         closed[index] = true;
      }
   }
};

#endif
