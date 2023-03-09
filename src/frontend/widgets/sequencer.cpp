#include "sequencer.h"
#include "extractor/mplaydef.h"
#include "imgui.h"


namespace frontend {

Sequencer::Sequencer(const Song* song) : song{song} {
  max_frame = 0;
  for (const auto& track : song->tracks) {
    max_frame = std::max(max_frame, track.length);
  }
}

int Sequencer::GetFrameMin() const {
  return 0;
}

int Sequencer::GetFrameMax() const {
  return max_frame;
}

int Sequencer::GetTimelineCount() const {
  return song->tracks.size();
}

int Sequencer::GetTimelineItemCount(int index) const {
  return song->tracks[index].events.size();
}

std::string Sequencer::GetTimelineLabel(int index) const {
  return "Track " + std::to_string(index);
}

void Sequencer::Get(int timeline, int index, int* start, int* end, unsigned int* color) const {
  const auto& event = song->tracks[timeline].events[index];
  if (start)
    *start = event.tick;
  if (end) {
    if (event.type == Event::Type::Note) {
      *end = event.tick + event.note.length;
    }
    else {
      *end = event.tick + 1;
    }
  }
  if (color) {
    switch (event.type) {
      case Event::Type::Note:
        *color = 0xffaa8080;
        break;
      default:
        *color = 0xff8080aa;
        break;
    }
  }
}

void Sequencer::DoubleClick(int timeline, int index) {
  selected_event = &song->tracks[timeline].events[index];
}

void Sequencer::Draw() {
  if (!ImGui::Begin("Sequencer", &show)) {
    ImGui::End();
    return;
  }

  ImSequencer::Sequencer(
     this, &current_frame, nullptr, &selected, &first_frame
  );

  if (selected_event) {
    ImGui::Text("Hi there!");
  }

  ImGui::End();
}

}