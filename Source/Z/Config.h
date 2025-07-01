//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace Z {

//! Configuration of interpreter features
struct Config
{
   uint8_t  interp_major_version{1};
   uint8_t  interp_minor_version{0};
   bool     status_line{true};       //!< Status line available
   bool     screen_splitting{true};  //!< Screen splitting available
   bool     var_pitch_font{false};   //!< variable pitch font the default
   bool     pictures{false};         //!< picture display available
   bool     sounds{false};           //!< sounds available
   bool     transcripting{false};    //!< transcripting starts on
   bool     undo{true};              //!< undo available
   bool     mouse{false};            //!< sounds available
   bool     menus{false};            //!< menus available
};

} // namespace Z

