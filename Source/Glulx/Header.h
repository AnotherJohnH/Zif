//-------------------------------------------------------------------------------
// Copyright (c) 2019 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "STB/Endian.h"

namespace Glulx {

//! Glulx header
struct Header
{
   uint8_t    magic[4];
   STB::Big16 version_major;
   uint8_t    version_minor;
   uint8_t    version_index;
   STB::Big32 ram_start;
   STB::Big32 ext_start;
   STB::Big32 end_mem;
   STB::Big32 stack_size;
   STB::Big32 start_func;
   STB::Big32 decoding_table;
   STB::Big32 checksum;
};

} // namespace Glulx

