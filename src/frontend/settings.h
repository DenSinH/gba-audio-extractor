#include "imgui.h"
#include <unordered_map>
#include <string>
#include <concepts>
#include "types.h"


class Settings {
public:
    template<typename T>
    void Set(const std::string& key, const T& value) {
        settings[key] = Serialize(value);
    }

    template<typename T>
    bool Read(const std::string& key, T& value) const {
        auto it = settings.find(key);
        if (it != settings.end()) {
            Deserialize(it->second, value);
            return true;
        }
        return false;
    }

    void SaveAll(ImGuiTextBuffer* out_buf) {
        for (const auto& pair : settings) {
            out_buf->appendf("%s=%s\n", pair.first.c_str(), pair.second.c_str());
        }
    }

private:
    template<typename T>
    std::string Serialize(const T& value) const {
        return std::to_string(value);
    }

    std::string Serialize(const std::string& value) const {
        return value;
    }

    template<typename T>
    void Deserialize(const std::string& str, T& value) const {
        value = static_cast<T>(std::stod(str));
    }

    void Deserialize(const std::string& str, std::string& value) const {
        value = str;
    }

    template<std::unsigned_integral T>
    void Deserialize(const std::string& str, T& value) const {
        value = static_cast<T>(std::stoul(str));
    }

    template<std::signed_integral T>
    void Deserialize(const std::string& str, T& value) const {
        value = static_cast<T>(std::stoi(str));
    }

    template<std::floating_point T>
    void Deserialize(const std::string& str, T& value) const {
        value = static_cast<T>(std::stod(str));
    }

private:
    std::unordered_map<std::string, std::string> settings;
};


void AddSettingsHandler(Settings& settings);
