#pragma once

#include "extractor/player.h"
#include "sequencer/ImSequencer.h"

namespace frontend {

struct Sequencer final : public ImSequencer::SequenceInterface {
  bool show = true;
  const Player* player;

  explicit Sequencer(const Player* player);

  void Draw();

private:
  u32 max_frame;
  int selected = -1;
  int first_frame = 0;

  const Event* selected_event = nullptr;

  int GetFrameMin() const final;
  int GetFrameMax() const final;
  int GetTimelineCount() const final;
  int GetTimelineItemCount(int index) const final;
  std::string GetTimelineLabel(int index) const final;
  void Get(int timeline, int index, int* start, int* end, unsigned int* color) const final;
  void DoubleClick(int timeline, int index) final;
};

}