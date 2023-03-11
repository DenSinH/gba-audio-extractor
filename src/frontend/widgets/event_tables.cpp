#include "event_tables.h"

namespace frontend {

const std::unordered_map<Event::Type, const char*> EventNames = {
    {Event::Type::Goto, "Goto"},
    {Event::Type::Tempo, "Tempo"},
    {Event::Type::VoiceChange, "VoiceChange"},
    {Event::Type::Controller, "Controller"},
    {Event::Type::Note, "Note"},
    {Event::Type::Fine, "Fine"},
};

const extern std::unordered_map<Controller::Type, const char*> ControllerEventNames = {
    { Controller::Type::VOL, "VOL" },
    { Controller::Type::PAN, "PAN" },
    { Controller::Type::BEND, "BEND" },
    { Controller::Type::BENDR, "BENDR" },
    { Controller::Type::LFOS, "LFOS" },
    { Controller::Type::LFODL, "LFODL" },
    { Controller::Type::MOD, "MOD" },
    { Controller::Type::MODT, "MODT" },
    { Controller::Type::TUNE, "TUNE" },
};

const std::array<const char*, 128> NoteNames = {
    "CnM2", "CsM2", "DnM2", "DsM2", "EnM2", "FnM2", "FsM2", "GnM2", "GsM2", "AnM2", "AsM2", "BnM2",
    "CnM", "CsM", "DnM", "DsM", "EnM", "FnM", "FsM", "GnM", "GsM", "AnM", "AsM", "BnM",
    "Cn0", "Cs0", "Dn0", "Ds0", "En0", "Fn0", "Fs0", "Gn0", "Gs0", "An0", "As0", "Bn0",
    "Cn1", "Cs1", "Dn1", "Ds1", "En1", "Fn1", "Fs1", "Gn1", "Gs1", "An1", "As1", "Bn1",
    "Cn2", "Cs2", "Dn2", "Ds2", "En2", "Fn2", "Fs2", "Gn2", "Gs2", "An2", "As2", "Bn2",
    "Cn3", "Cs3", "Dn3", "Ds3", "En3", "Fn3", "Fs3", "Gn3", "Gs3", "An3", "As3", "Bn3",
    "Cn4", "Cs4", "Dn4", "Ds4", "En4", "Fn4", "Fs4", "Gn4", "Gs4", "An4", "As4", "Bn4",
    "Cn5", "Cs5", "Dn5", "Ds5", "En5", "Fn5", "Fs5", "Gn5", "Gs5", "An5", "As5", "Bn5",
    "Cn6", "Cs6", "Dn6", "Ds6", "En6", "Fn6", "Fs6", "Gn6", "Gs6", "An6", "As6", "Bn6",
    "Cn7", "Cs7", "Dn7", "Ds7", "En7", "Fn7", "Fs7", "Gn7", "Gs7", "An7", "As7", "Bn7",
    "Cn8", "Cs8", "Dn8", "Ds8", "En8", "Fn8", "Fs8", "Gn8",
};

}
