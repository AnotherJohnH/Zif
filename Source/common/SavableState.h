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

#ifndef IF_SAVABLE_STATE_H
#define IF_SAVABLE_STATE_H

#include <cstdio>

#include "common/Story.h"
#include "common/Quetzal.h"
#include "common/State.h"

namespace IF {

//! Machine state class for an interactive fiction VM with save/restore and undo
class SavableState : public State
{
public:
   //! Get save filename
   static std::string saveFilename(const std::string& dir,
                                   const std::string& story_filename,
                                   const std::string& save_id = "")
   {
      std::string path = dir;
      path += '/';
      path += story_filename;
      if (save_id != "")
      {
         path += '_';
         path += save_id;
      }
      path += ".qzl";
      return path;
   }

   static bool saveFileExists(const std::string& dir,
                              const std::string& story_filename,
                              const std::string& save_id = "")
   {
      std::string save_filename = saveFilename(dir, story_filename, save_id);

      FILE* fp = fopen(save_filename.c_str(), "r");
      if (fp == nullptr) return false;
      fclose(fp);
      return true;
   }

   SavableState(const IF::Story&   story_,
                const std::string& save_dir_,
                unsigned           num_undo_,
                uint32_t           initial_rand_seed_,
                Stack::Offset      stack_size_)
      : IF::State(story_, initial_rand_seed_, stack_size_)
      , save_dir(save_dir_)
   {
      undo.resize(num_undo_);
   }

   //! Save the dynamic state to a file
   bool save(const std::string& name = "")
   {
      pushContext();
      save_file.encode(story, *this);
      popContext();

      // Make sure the save directory exists
      (void) PLT::File::createDir(save_dir.c_str());

      std::string path = getSaveFilename(name);
      return save_file.write(path);
   }

   //! Restore the dynamic state from a save file
   bool restore(const std::string& name = "")
   {
      std::string path = getSaveFilename(name);

      if (save_file.read(path) && save_file.decode(story, *this))
      {
         popContext();
         return true;
      }

      return false;
   }

   //! Save the dynamic state into the undo buffer
   bool saveUndo()
   {
      if (undo.size() == 0) return false;

      pushContext();
      undo[undo_next].encode(story, *this);
      popContext();

      undo_next = (undo_next + 1) % undo.size();
      if (undo_next == undo_oldest)
      {
         undo_oldest = (undo_oldest + 1) % undo.size();
      }

      return true;
   }

   //! Restore the dynamic state from the undo buffer
   bool restoreUndo()
   {
      if (undo_next == undo_oldest) return false;

      undo_next = undo_next == 0 ? undo.size() - 1
                                 : undo_next - 1;

      undo[undo_next].decode(story, *this);
      popContext();

      return true;
   }

   //! Save dynamic registers on the stack
   virtual void pushContext() = 0;

   //! Restore dynamic registers from the stack
   virtual void popContext() = 0;

private:
   std::string              save_dir;
   IF::Quetzal              save_file;
   std::vector<IF::Quetzal> undo;
   unsigned                 undo_oldest{0};
   unsigned                 undo_next{0};

   //! Get save filename
   std::string getSaveFilename(const std::string& name)
   {
      return saveFilename(save_dir, story.getFilename(), name);
   }
};

} // namespace IF

#endif
