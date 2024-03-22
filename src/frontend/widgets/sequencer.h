#pragma once

#include "extractor/mp2k_driver.h"
#include "sequencer/ImSequencer.h"

namespace frontend {

struct Sequencer final : public ImSequencer::SequenceInterface {
  Mp2kDriver* driver;

  explicit Sequencer(Mp2kDriver* driver) : driver{driver} {
    
  };

  void Draw();
  void SelectSong();

private:
  bool was_paused = false;
  bool dragging_frame = false;
  u32 max_frame = 0;
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