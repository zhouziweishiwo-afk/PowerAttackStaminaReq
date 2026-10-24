#include "PCH.h"
#include "Hook.h"
#include "Settings.h"

namespace
{
    void OnMessage(SKSE::MessagingInterface::Message* a_message)
    {
        if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
            Settings::Load();
            Hook::Install();
        }
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging) {
        logger::critical("messaging interface unavailable, plugin will not load");
        return false;
    }

    messaging->RegisterListener(OnMessage);
    logger::info("PowerAttackStaminaReq loaded");
    return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
    SKSE::PluginVersionData v{};
    v.PluginVersion(1);
    v.PluginName("PowerAttackStaminaReq");
    v.AuthorName("");
    v.UsesAddressLibrary(true);   // 依赖 Address Library for SKSE Plugins
    v.HasNoStructUse(true);       // 不使用游戏结构体偏移，避免新版 exe 的版本校验
    return v;
}();
