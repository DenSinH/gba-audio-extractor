#pragma once

#include "extractor/mplaydef.h"
#include "extractor/event.h"

#include <unordered_map>
#include <array>

namespace frontend {

const extern std::unordered_map<Event::Type, const char*> EventNames;
const extern std::unordered_map<Controller::Type, const char*> ControllerEventNames;
const extern std::array<const char*, 128> NoteNames;

}