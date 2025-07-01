//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

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

