#include "sequencer.h"
#include "extractor/mplaydef.h"
#include "imgui.h"
#include "event_tables.h"


namespace frontend {

Sequencer::Sequencer(const Player* player) : player{player} {
  max_frame = 0;
  for (const auto& track : player->song->tracks) {
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
  return player->song->tracks.size();
}

int Sequencer::GetTimelineItemCount(int index) const {
  return player->song->tracks[index].events.size();
}

std::string Sequencer::GetTimelineLabel(int index) const {
  return "Track " + std::to_string(index + 1);
}

void Sequencer::Get(int timeline, int index, int* start, int* end, unsigned int* color) const {
  const auto& event = player->song->tracks[timeline].events[index];
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
  selected_event = &player->song->tracks[timeline].events[index];
}

void Sequencer::Draw() {
  if (!ImGui::Begin("Sequencer", &show)) {
    ImGui::End();
    return;
  }
  int current_frame = player->GetCurrentTick();
  ImSequencer::Sequencer(
     this, &current_frame, nullptr, &selected, &first_frame
  );

  if (selected_event) {
    ImGui::Text("Event type: %s", EventNames.at(selected_event->type));
    if (selected_event->type == Event::Type::Note) {
      ImGui::Text("Note:     %s", NoteNames.at(selected_event->note.key));
      ImGui::Text("Duration: %d", selected_event->note.length);
      ImGui::Text("Velocity: %d", selected_event->note.velocity);
    }
    else if (selected_event->type == Event::Type::Controller) {
      ImGui::Text("Controller: %s", ControllerEventNames.at(selected_event->controller.type));
    }
    else if (selected_event->type == Event::Type::VoiceChange) {
      ImGui::Text("Voice: %d", selected_event->voice_change.voice);
    }
  }

  ImGui::End();
}

}