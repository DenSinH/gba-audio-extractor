// https://github.com/CedricGuillemet/ImGuizmo
// v 1.89 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once

#include <cstddef>
#include <string>

struct ImDrawList;
struct ImRect;
namespace ImSequencer
{
   enum SEQUENCER_OPTIONS
   {
      SEQUENCER_EDIT_NONE = 0,
      SEQUENCER_EDIT_STARTEND = 1 << 1,
      SEQUENCER_CHANGE_FRAME = 1 << 3,
      SEQUENCER_ADD = 1 << 4,
      SEQUENCER_DEL = 1 << 5,
      SEQUENCER_COPYPASTE = 1 << 6,
      SEQUENCER_EDIT_ALL = SEQUENCER_EDIT_STARTEND | SEQUENCER_CHANGE_FRAME
   };

   struct SequenceInterface
   {
      bool focused = false;
      virtual int GetFrameMin() const = 0;
      virtual int GetFrameMax() const = 0;
      virtual int GetTimelineCount() const = 0;
      virtual int GetTimelineItemCount(int index) const = 0;

      virtual std::string GetTimelineLabel(int /*index*/) const { return ""; }
      virtual const char* GetCollapseFmt() const { return "%d Frames / %d timelines"; }

      virtual void Get(int timeline, int index, int* start, int* end, unsigned int* color) const = 0;

      virtual size_t GetCustomHeight(int timeline) { return 0; }
      virtual void DoubleClick(int timeline, int index) {}
      virtual void RightClick(int timeline) { }
      virtual unsigned int GetTimelineColor(int timeline) const { return 0; }
      virtual void CustomDraw(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*legendRect*/, const ImRect& /*clippingRect*/, const ImRect& /*legendClippingRect*/) {}
      virtual void CustomDrawCompact(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*clippingRect*/) {}

      virtual ~SequenceInterface() = default;
   };

   // return true if selection is made
   bool Sequencer(SequenceInterface* sequence, int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame);
}