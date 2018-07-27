//------------------------------------------------------------------------------
// Copyright (c) 2016-2017 John D. Haughton
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

#ifndef Z_OPTIONS_H
#define Z_OPTIONS_H

#include "STB/Option.h"

//! Command line options
struct ZOptions
{
   STB::Option<bool>        print{  'p', "print",   "Print output to \"print.log\""};
   STB::Option<bool>        info{   0,   "info",    "Report information messages"};
   STB::Option<bool>        warn{   0,   "warn",    "Report warning messages"};
   STB::Option<unsigned>    width{  'w', "width",   "Override output width", 0};
   STB::Option<bool>        batch{  'b', "batch",   "Batch mode, disable output to screen"};
   STB::Option<bool>        key{    'k', "key",     "Log key presses to \"key.log\""};
   STB::Option<const char*> input{  'i', "input",   "Read keyboard input from a file"};
   STB::Option<bool>        trace{  'T', "trace",   "Trace execution"};
   STB::Option<unsigned>    seed{   'S', "seed",    "Initial random seed", 0};
   STB::Option<unsigned>    undo{   'u', "undo",    "Number of undo buffers", 2};
   STB::Option<const char*> save{   's', "save",    "Set directory for save files", "save"};
   STB::Option<bool>        restore{'r', "restore", "Start game at last save"};
};

#endif
