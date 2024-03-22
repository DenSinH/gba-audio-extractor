#include "imgui.h"
#include "imgui_internal.h"
#include <unordered_map>
#include <string>
#include "settings.h"

#define SETTINGS_NAME "User Settings"


static void SettingsHandler_ClearAll(ImGuiContext*, ImGuiSettingsHandler*) {

}

static void* SettingsHandler_ReadOpen(ImGuiContext* /*ctx*/, ImGuiSettingsHandler* handler, const char* /*name*/) {
    return handler->UserData;
}

static bool ParseSettingLine(const char* line, std::string& key, std::string& value) {
    const char* equalPos = strchr(line, '=');
    if (!equalPos)
        return false;

    key.assign(line, equalPos - line);
    value.assign(equalPos + 1);

    return true;
}

static void SettingsHandler_ReadLine(ImGuiContext* /*ctx*/, ImGuiSettingsHandler* handler, void* entry, const char* line) {
    Settings* settings = static_cast<Settings*>(entry);
    if (!settings)
        return;

    std::string key, value;
    if (ParseSettingLine(line, key, value)) {
        settings->Set(key, value);
    }
}

static void SettingsHandler_ApplyAll(ImGuiContext* ctx, ImGuiSettingsHandler*) {
    // Optional: Handle applying settings if needed
}

static void SettingsHandler_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf) {    
    out_buf->appendf("[%s][Global]\n", SETTINGS_NAME);
    Settings* settings = static_cast<Settings*>(handler->UserData);
    if (settings)
        settings->SaveAll(out_buf);
}

void AddSettingsHandler(Settings& settings) {
    ImGuiSettingsHandler settingsHandler;
    settingsHandler.TypeName = SETTINGS_NAME;
    settingsHandler.TypeHash = ImHashStr(SETTINGS_NAME);
    settingsHandler.ClearAllFn = SettingsHandler_ClearAll;
    settingsHandler.ReadOpenFn = SettingsHandler_ReadOpen;
    settingsHandler.ReadLineFn = SettingsHandler_ReadLine;
    settingsHandler.ApplyAllFn = SettingsHandler_ApplyAll;
    settingsHandler.WriteAllFn = SettingsHandler_WriteAll;
    settingsHandler.UserData = static_cast<void*>(&settings);
    ImGui::AddSettingsHandler(&settingsHandler);
}
