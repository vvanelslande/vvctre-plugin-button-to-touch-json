// Copyright 2020 Valentin Vanelslande
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <nlohmann/json.hpp>
#include <whereami.h>

#include "string_util.h"

#ifdef _WIN32
#define VVCTRE_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define VVCTRE_PLUGIN_EXPORT extern "C"
#endif

static const char* required_function_names[] = {
    "vvctre_button_device_new",
    "vvctre_button_device_get_state",
    "vvctre_set_custom_touch_state",
    "vvctre_use_real_touch_state",
};

using vvctre_button_device_new_t = void* (*)(void* plugin_manager, const char* params);
using vvctre_button_device_get_state_t = bool (*)(void* device);
using vvctre_set_custom_touch_state_t = void (*)(void* core, float x, float y, bool pressed);
using vvctre_use_real_touch_state_t = void (*)(void* core);

static vvctre_button_device_new_t vvctre_button_device_new;
static vvctre_button_device_get_state_t vvctre_button_device_get_state;
static vvctre_set_custom_touch_state_t vvctre_set_custom_touch_state;
static vvctre_use_real_touch_state_t vvctre_use_real_touch_state;

static void* core = nullptr;
static void* plugin_manager = nullptr;

struct Button {
    void* device = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    bool pressed = false;
};
static std::vector<Button> buttons;

VVCTRE_PLUGIN_EXPORT int GetRequiredFunctionCount() {
    return 4;
}

VVCTRE_PLUGIN_EXPORT const char** GetRequiredFunctionNames() {
    return required_function_names;
}

VVCTRE_PLUGIN_EXPORT void PluginLoaded(void* core_, void* plugin_manager_,
                                       void* required_functions[]) {
    core = core_;
    plugin_manager = plugin_manager_;
    vvctre_button_device_new = (vvctre_button_device_new_t)required_functions[0];
    vvctre_button_device_get_state = (vvctre_button_device_get_state_t)required_functions[1];
    vvctre_set_custom_touch_state = (vvctre_set_custom_touch_state_t)required_functions[2];
    vvctre_use_real_touch_state = (vvctre_use_real_touch_state_t)required_functions[3];
}

VVCTRE_PLUGIN_EXPORT void InitialSettingsOpening() {
    int length = wai_getExecutablePath(nullptr, 0, nullptr);
    std::string vvctre_folder(length, '\0');
    int dirname_length = 0;
    wai_getExecutablePath(&vvctre_folder[0], length, &dirname_length);
    vvctre_folder = vvctre_folder.substr(0, dirname_length);

    std::ifstream file;
#ifdef _MSC_VER
    file.open(Common::UTF8ToUTF16W(vvctre_folder + "\\button-to-touch.json"));
#else
    file.open(vvctre_folder + "/button-to-touch.json");
#endif

    if (!file.fail()) {
        std::ostringstream oss;
        oss << file.rdbuf();

        const nlohmann::json json = nlohmann::json::parse(oss.str());

        for (const nlohmann::json& json_button : json) {
            Button button{};
            button.device = vvctre_button_device_new(
                plugin_manager, json_button["params"].get<std::string>().c_str());
            button.x = json_button["x"].get<int>() / 319;
            button.y = json_button["y"].get<int>() / 239;
            buttons.push_back(button);
        }
    }
}

VVCTRE_PLUGIN_EXPORT void AfterSwapWindow() {
    for (Button& button : buttons) {
        const bool pressed = vvctre_button_device_get_state(button.device);
        if (button.pressed && !pressed) {
            vvctre_use_real_touch_state(core);
            button.pressed = false;
        } else if (!button.pressed && pressed) {
            vvctre_set_custom_touch_state(core, button.x, button.y, true);
            button.pressed = true;
        }
    }
}
