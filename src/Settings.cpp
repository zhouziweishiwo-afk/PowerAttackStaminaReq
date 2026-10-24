#include "PCH.h"
#include "Settings.h"

#include <fstream>
#include <string>

namespace
{
    Settings::Config g_config{};

    std::string Trim(const std::string& a_in)
    {
        const auto first = a_in.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            return {};
        }
        const auto last = a_in.find_last_not_of(" \t\r\n");
        return a_in.substr(first, last - first + 1);
    }

    bool AsBool(const std::string& a_in, bool a_default)
    {
        if (a_in.empty()) { return a_default; }
        if (a_in == "1" || a_in == "true" || a_in == "True" || a_in == "yes") { return true; }
        if (a_in == "0" || a_in == "false" || a_in == "False" || a_in == "no") { return false; }
        return a_default;
    }

    int AsInt(const std::string& a_in, int a_default)
    {
        try {
            return std::stoi(a_in);
        } catch (...) {
            return a_default;
        }
    }
}

namespace Settings
{
    const Config& Get() noexcept { return g_config; }

    void Load()
    {
        constexpr auto path = "Data/SKSE/Plugins/PowerAttackStaminaReq.ini";

        std::ifstream file(path);
        if (!file.is_open()) {
            logger::info("ini not found ({}), using defaults", path);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            const auto trimmed = Trim(line);
            if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#' || trimmed[0] == '[') {
                continue;
            }
            const auto eq = trimmed.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            const auto key = Trim(trimmed.substr(0, eq));
            const auto val = Trim(trimmed.substr(eq + 1));

            if      (key == "Enabled")             { g_config.enabled      = AsBool(val, true); }
            else if (key == "ReverseMouseButtons") { g_config.reverseMouse = AsBool(val, false); }
            else if (key == "LeftButton")          { g_config.leftButton   = AsInt(val, 280); }
            else if (key == "RightButton")         { g_config.rightButton  = AsInt(val, 281); }
            else if (key == "DebugLog")            { g_config.debugLog     = AsBool(val, false); }
        }

        logger::info("settings: enabled={} left={} right={} reverseMouse={}",
                     g_config.enabled, g_config.leftButton, g_config.rightButton, g_config.reverseMouse);
    }
}
