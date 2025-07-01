//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "STB/Option.h"

//! Command line options
struct Options
{
   STB::Option<bool>        info{    0,   "info",     "Report information messages"};
   STB::Option<bool>        warn{    0,   "warn",     "Report warning messages"};
   STB::Option<unsigned>    width{   'w', "width",    "Override output width", 0};
   STB::Option<bool>        batch{   'b', "batch",    "Batch mode, disable output to screen"};
   STB::Option<bool>        trace{   'T', "trace",    "Trace execution to \"trace.log\""};
   STB::Option<bool>        print{   'p', "print",    "Print output to \"print.log\""};
   STB::Option<bool>        key{     'k', "key",      "Log key presses to \"key.log\""};
   STB::Option<const char*> input{   'i', "input",    "Read keyboard input from a file"};
   STB::Option<unsigned>    seed{    'S', "seed",     "Initial random number seed", 0};
   STB::Option<unsigned>    undo{    'u', "undo",     "Number of undo buffers", 4};
   STB::Option<const char*> save_dir{'s', "save-dir", "Directory for save files", "Saves"};
};

