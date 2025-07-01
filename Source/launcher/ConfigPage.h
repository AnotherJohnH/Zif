//-------------------------------------------------------------------------------
// Copyright (c) 2016 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdlib>

#include "STB/Oil.h"

#include "Page.h"
#include "SelectorItem.h"

struct TermConfig : public STB::Oil<TermConfig>
{
   unsigned font_size{18};
   unsigned border_pixels{0};
   unsigned line_space{0};
   unsigned sleep{0};
   bool     inverse_video{false};
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
   MOIL(inverse_video);
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

      inverse.set(config.inverse_video ? "ON" : "off");

      if (config.sleep == 0)
      {
         sleep.set("off");
      }
      else
      {
         sleep.setInt(config.sleep);
      }
   }

   void setTermDevice(TRM::Device& term_)
   {
      term = &term_;
      term->ioctl(TRM::Device::IOCTL_TERM_SLEEP_IMAGE, "Images/saver.png");

      configTerminal();
   }

private:
   TermConfig   config;
   TRM::Device* term{nullptr};

   SelectorItem size{   this,  7, 3, "Font size    ", "9,12,15,18"};
   SelectorItem border{ this,  8, 3, "Border       ", "0,4,8,16", "pixels"};
   SelectorItem space{  this,  9, 3, "Line space   ", "0,1,2,3", "pixels"};

   SelectorItem sleep{  this, 11, 3, "Screen save  ", "off,1,5,10", "min"};

   SelectorItem inverse{this, 13, 3, "Inverse Video", "off,ON"};
#ifndef PROJ_TARGET_Kindle3
   SelectorItem colour{ this, 14, 3, "Colours      ",
                        "Green Phosphor,Amber Phosphor,Blue Phosphor,Old Paper,White"};
#endif

   //! update the terminal configuration
   void configTerminal()
   {
      term->ioctl(TRM::Device::IOCTL_TERM_PALETTE, 0,
                  config.inverse_video ? config.fg_colour : config.bg_colour);

      term->ioctl(TRM::Device::IOCTL_TERM_PALETTE, 1,
                  config.inverse_video ? config.bg_colour : config.fg_colour);

      term->ioctl(TRM::Device::IOCTL_TERM_BORDER,      config.border_pixels);
      term->ioctl(TRM::Device::IOCTL_TERM_LINE_SPACE,  config.line_space);
      term->ioctl(TRM::Device::IOCTL_TERM_FONT_SIZE,   config.font_size);
      term->ioctl(TRM::Device::IOCTL_TERM_SLEEP,       config.sleep * 60);

      curses.init();
   }

   virtual bool show(const std::string& program) override
   {
      bool status = Page::show(program);

      curses.mvaddstr(4, 3, "Columns       : ");
      curses.addstr(std::to_string(curses.cols).c_str());
      curses.mvaddstr(5, 3, "Lines         : ");
      curses.addstr(std::to_string(curses.lines).c_str());

      return status;
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
#ifndef PROJ_TARGET_Kindle3
      else if (active == &colour)
      {
         inverse.set("off");

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
#endif
      else if (active == &inverse)
      {
         bool inverse_video = value == "ON";

         if(config.inverse_video != inverse_video)
         {
            config.inverse_video = inverse_video;
         }
      }
      else if (active == &sleep)
      {
         if (value == "off")
         {
            config.sleep = 0;
         }
         else
         {
            config.sleep = sleep.getInt();
         }
      }

      configTerminal();

      config.write();

      return false;
   }
};

