#pragma once

#include "extractor/player.h"
#include "sequencer/ImSequencer.h"

namespace frontend {

struct Sequencer final : public ImSequencer::SequenceInterface {
  bool show = true;
  Player* player;

  explicit Sequencer(Player* player);

  void Draw();

private:
  bool was_paused;
  bool dragging_frame = false;
  u32 max_frame;
  int selected = -1;
  int current_tick = 0;
  int first_frame = 0;

  const Event* selected_event = nullptr;

  int GetFrameMin() const final;
  int GetFrameMax() const final;
  int GetTimelineCount() const final;
  int GetTimelineItemCount(int index) const final;
  unsigned int GetTimelineColor(int timeline) const final;
  std::string GetTimelineLabel(int index) const final;
  void Get(int timeline, int index, int* start, int* end, unsigned int* color) const final;
  void DoubleClick(int timeline, int index) final;
  void RightClick(int timeline) final;
};

}