#include "frontend.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <iostream>
#include <cmath>
#include <SDL.h>
#include "extractor/mp2k_driver.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include "glad/glad.h"
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include "settings.h"
#include "widgets/sequencer.h"
#include "modules/imgui-filebrowser/imfilebrowser.h"
#include "constants.h"

namespace frontend {

static SDL_Window* window;
static SDL_GLContext gl_context;
static const char* glsl_version = "#version 130";

static Mp2kDriver* driver = nullptr;

static Settings settings = {};
static std::string filename = {};
static float volumesq = 0.25f;
static int songidx = 0;


static void AudioCallback(void*, u8* _stream, int len) {
  float* stream = (float*)_stream;

  if (driver && driver->player) {
    for (int i = 0; i < len / sizeof(float); i += 2) {
      const auto sample = driver->player->GetNextSample();
      stream[i]     = 2 * std::sqrt(volumesq) * sample.left;
      stream[i + 1] = 2 * std::sqrt(volumesq) * sample.right;
    }
  }
  else {
    std::memset(stream, 0, len);
  }
}

static SDL_AudioSpec audio_spec = {
    .freq = int(SampleRate),
    .format = AUDIO_F32,
    .channels = 2,
    .samples = 512,
    .callback = AudioCallback
};

static void InitSDLVideo() {

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  window = SDL_CreateWindow("Audio Extractor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  gl_context = SDL_GL_CreateContext(window);
  int err = gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress) == 0;
  if (err) {
    std::printf("Failed to create OpenGL context!\n");
    exit(1);
  }

  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync
}

static void InitSDLAudio() {
  if ( SDL_OpenAudio(&audio_spec, NULL) < 0 ) {
    printf("Unable to open audio: %s\n", SDL_GetError());
    exit(1);
  }
  SDL_PauseAudio(0);
}

static void InitSDL() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
  {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  InitSDLVideo();
  InitSDLAudio();
}

static void InitImGui() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

static void InitSettings() {
  AddSettingsHandler(settings);
  ImGuiIO& io = ImGui::GetIO(); 
  if (io.IniFilename)
    ImGui::LoadIniSettingsFromDisk(io.IniFilename);
  settings.Read("volume", volumesq);
  settings.Read("filename", filename);
  settings.Read("songidx", songidx);
}

static void SaveSettings() {
  ImGuiIO& io = ImGui::GetIO(); 
  if (io.IniFilename)
    ImGui::SaveIniSettingsToDisk(io.IniFilename);
}

static void Destroy() {
  SaveSettings();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}


int Run(Mp2kDriver* _driver) {
  driver = _driver;
  InitSDL();
  InitImGui();
  InitSettings();

  ImGuiIO& io = ImGui::GetIO();
  auto sequencer = Sequencer(driver);
  auto fdialog = ImGui::FileBrowser();
  fdialog.SetTitle("Choose ROM");
  fdialog.SetTypeFilters({".gba", ".bin"});
  {
    std::string fdialogpwd{};
    settings.Read("fdialogpwd", fdialogpwd);
    if (!fdialogpwd.empty()) {
      fdialog.SetPwd(fdialogpwd);
    }
  }

  if (!filename.empty()) {
    driver->Init(filename);
    if (songidx < driver->song_count) {
      driver->SelectSong(songidx);
      sequencer.SelectSong();
    }
  }

  // Main loop
  bool done = false;
#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = NULL;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!done)
#endif
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (fdialog.IsOpened()) {
      ImGui::BeginDisabled();
    }

    static constexpr int FileNameHeight = 25;
    static constexpr int ControlHeight = 50;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, FileNameHeight));
    ImGui::Begin("Filename", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    {
      ImGui::Text("%s", filename.c_str());
    }
    ImGui::End();
    ImGui::SetNextWindowPos(ImVec2(0, FileNameHeight));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, ControlHeight));
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    {
      if (ImGui::Button("Choose file")) {
        fdialog.Open();
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(150);
      if (ImGui::InputInt(("Song ID / " + std::to_string(driver->song_count)).c_str(), &songidx)) {
        songidx = std::clamp(songidx, 0, driver->song_count);
        settings.Set("songidx", songidx);
        if (songidx < driver->song_count) {
          driver->SelectSong(songidx);
          sequencer.SelectSong();
        }
      }
      if (!driver->player) {
        ImGui::BeginDisabled();
      }
      ImGui::SameLine();
      if (ImGui::Button("Pause/Play")) {
        driver->player->paused ^= true;
      }
      if (!driver->player) {
        ImGui::EndDisabled();
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(200);
      if (ImGui::SliderFloat("Volume", &volumesq, 0.0, 1.0)) {
        settings.Set("volume", volumesq);
      }
    }
    ImGui::End();
    ImGui::SetNextWindowPos(ImVec2(0, FileNameHeight + ControlHeight));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0.5 * io.DisplaySize.y));
    sequencer.Draw();

    ImGui::SetNextWindowPos(ImVec2(0.6 * io.DisplaySize.x, FileNameHeight + ControlHeight + 0.5 * io.DisplaySize.y));
    ImGui::SetNextWindowSize(ImVec2(0.4 * io.DisplaySize.x, io.DisplaySize.y - (FileNameHeight + ControlHeight + 0.5 * io.DisplaySize.y)));
    ImGui::Begin("Voice Envelope", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    const auto* e = sequencer.SelectedEvent();
    if (e && e->type == Event::Type::Note) {
      // const auto* voice = e->note.voice
    }
    ImGui::End();    

    if (fdialog.IsOpened()) {
      ImGui::EndDisabled();
    }
    fdialog.Display();
    if (fdialog.HasSelected()) {
      filename = fdialog.GetSelected().string();
      driver->Init(filename);
      settings.Set("filename", filename);
      songidx = 0;
      driver->SelectSong(songidx);
      sequencer.SelectSong();
      settings.Set("fdialogpwd", fdialog.GetPwd().string());
      fdialog.ClearSelected();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif

  Destroy();

  return 0;
}

}
