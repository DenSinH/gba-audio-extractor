#include "sequencer.h"
#include "extractor/mplaydef.h"
#include "imgui.h"
#include "event_tables.h"


namespace frontend {


void Sequencer::SelectSong() {
  max_frame = 0;
  first_frame = driver->player->GetCurrentTick();
  selected_event = nullptr;
  was_paused = driver->player->paused;
  for (const auto& track : driver->player->song.tracks) {
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
  assert(driver && driver->player);
  return driver->player->song.tracks.size();
}

int Sequencer::GetTimelineItemCount(int index) const {
  assert(driver && driver->player);
  return driver->player->song.tracks[index].events.size();
}

std::string Sequencer::GetTimelineLabel(int index) const {
  assert(driver && driver->player);
  auto name = "Track " + std::to_string(index + 1);
  const auto* note = driver->player->GetTrackNote(index);
  if (note) {
    name += ": " + std::string(NoteNames[note->key]);
  }
  else {
    name += "     ";
  }
  return name;
}

unsigned int Sequencer::GetTimelineColor(int timeline) const {
  assert(driver && driver->player);
  if (driver->player->GetTrackEnable(timeline)) {
    return 0;
  }
  return 0xc08080aa;
}

void Sequencer::Get(int timeline, int index, int* start, int* end, unsigned int* color) const {
  assert(driver && driver->player);
  const auto& event = driver->player->song.tracks[timeline].events[index];
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
  assert(driver && driver->player);
  selected_track = timeline;
  selected_event = &driver->player->song.tracks[timeline].events[index];
}

void Sequencer::RightClick(int timeline) {
  assert(driver && driver->player);
  driver->player->ToggleTrackEnable(timeline);
}

void Sequencer::Draw() {
  const auto io = ImGui::GetIO();
  
  ImGui::Begin("Sequencer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
  if (!(driver && driver->player)) {
    ImGui::End();
    return;
  }

  const int player_tick = driver->player->GetCurrentTick();
  current_tick = player_tick;
  ImSequencer::Sequencer(
     this, &current_tick, nullptr, &selected, &first_frame
  );

  if (current_tick != player_tick) {
    if (!dragging_frame) {
      // start dragging frame
      was_paused = driver->player->paused;
      dragging_frame = true;
    }
    driver->player->paused = true;
  }

  if (dragging_frame && !io.MouseDown[0]) {
    // stop dragging frame
    dragging_frame = false;
    driver->player->SkipToTick(current_tick);
    driver->player->paused = was_paused;
  }

  if (selected_event) {
    ImGui::Text("Event type: %s", EventNames.at(selected_event->type));
    if (selected_event->type == Event::Type::Note) {
      ImGui::Text("Note: %s [L %d, v %d]", NoteNames.at(selected_event->note.key), selected_event->note.length, selected_event->note.velocity);
    }
    else if (selected_event->type == Event::Type::Controller) {
      ImGui::Text("Controller: %s %02x", ControllerEventNames.at(selected_event->controller.type), selected_event->controller.val);
    }
    else if (selected_event->type == Event::Type::VoiceChange) {
      ImGui::Text("Voice: %d", selected_event->voice_change.voice);
    }
  }

  ImGui::End();
}

}