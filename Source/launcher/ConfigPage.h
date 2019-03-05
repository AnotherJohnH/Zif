//------------------------------------------------------------------------------
// Copyright (c) 2016-2019 John D. Haughton
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

#ifndef CONFIG_PAGE_H
#define CONFIG_PAGE_H

#include <cstdlib>

#include "STB/Oil.h"

#include "Page.h"
#include "SelectorItem.h"

struct TermConfig : public STB::Oil<TermConfig>
{
   unsigned font_size{18};
   unsigned border_pixels{0};
   unsigned line_space{0};
   bool     invert_video{false};
   unsigned sleep{1};
#ifdef PROJ_TARGET_Kindle3
   uint32_t bg_colour{0xFFFFFF};
   uint32_t fg_colour{0x000000};
#else
   uint32_t bg_colour{0xF0F0E0};
   uint32_t fg_colour{0x382800};
#endif
};

BOIL(TermConfig)
{
   MOIL(font_size);
   MOIL(border_pixels);
   MOIL(line_space);
   MOIL(sleep);
   MOIL(bg_colour); FOIL(bg_colour, HEX);
   MOIL(fg_colour); FOIL(fg_colour, HEX);
}
EOIL(TermConfig)

//! Manage the settings page
class ConfigPage : public Page
{
public:
   ConfigPage(TRM::Curses& curses_)
      : Page(curses_, "Settings")
   {
      if (config.exists())
      {
         config.read();
      }

      size.setInt(config.font_size);
      border.setInt(config.border_pixels);
      space.setInt(config.line_space);
      sleep.setInt(config.sleep);
   }

   void setTermDevice(TRM::Device& term_)
   {
      term = &term_;
      configTerminal();
   }

private:
   TermConfig   config;
   TRM::Device* term{nullptr};

   SelectorItem size{   this,  4, 3, "Font size ", "9,12,15,18"};
   SelectorItem border{ this,  5, 3, "Border    ", "0,4,8,16"};
   SelectorItem space{  this,  6, 3, "Line space", "0,1,2,3"};
   SelectorItem colour{ this,  8, 3, "Colours   ", 
                        "Green Phosphor,Amber Phosphor,Blue Phosphor,Old Paper,White"};
   SelectorItem video{  this,  9, 3, "Video     ", "Normal,Inverse"};
   SelectorItem sleep{  this, 11, 3, "Sleep     ", "0,1,5,10"};

   //! update the terminal configuration
   void configTerminal()
   {
      term->ioctl(TRM::Device::IOCTL_TERM_PALETTE, 0, config.bg_colour);
      term->ioctl(TRM::Device::IOCTL_TERM_PALETTE, 1, config.fg_colour);
      term->ioctl(TRM::Device::IOCTL_TERM_BORDER, config.border_pixels);
      term->ioctl(TRM::Device::IOCTL_TERM_LINE_SPACE, config.line_space);
      term->ioctl(TRM::Device::IOCTL_TERM_FONT_SIZE, config.font_size);
      term->ioctl(TRM::Device::IOCTL_TERM_SLEEP, config.sleep * 60);

      curses.init();
   }

   virtual bool select(std::string& cmd, std::string& value) override
   {
      active->select(cmd, value);

      if (active == &size)
      {
         config.font_size = size.getInt();
      }
      else if (active == &border)
      {
         config.border_pixels = border.getInt();
      }
      else if (active == &space)
      {
         config.line_space = space.getInt();
      }
      else if (active == &colour)
      {
         if(value == "Green Phosphor")
         {
            config.bg_colour = 0x40FF40;
            config.fg_colour = 0x000000;
         }
         else if(value == "Amber Phosphor")
         {
            config.bg_colour = 0xFFC000;
            config.fg_colour = 0x000000;
         }
         else if(value == "Blue Phosphor")
         {
            config.bg_colour = 0xD0F0FF;
            config.fg_colour = 0x000000;
         }
            else if(value == "Old Paper")
         {
            config.bg_colour = 0xF0F0E0;
            config.fg_colour = 0x382800;
         }
         else if(value == "White")
         {
            config.bg_colour = 0xFFFFFF;
            config.fg_colour = 0x000000;
         }
      }
      else if (active == &video)
      {
         bool invert = value == "Inverse";

         if(config.invert_video != invert)
         {
            config.invert_video = invert;
            std::swap(config.bg_colour, config.fg_colour);
         }
      }
      else if (active == &sleep)
      {
         config.sleep = sleep.getInt();
      }

      configTerminal();

      config.write();

      return false;
   }
};

#endif
