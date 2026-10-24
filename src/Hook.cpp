#include "PCH.h"
#include "Hook.h"
#include "Cost.h"
#include "Settings.h"

namespace
{
    // x64 下成员函数的 this 走 rcx，与普通函数的第一个参数一致，
    // 因此可以用普通函数指针调用 AttackBlockHandler::ProcessButton。
    using ProcessButtonFn = void (*)(RE::AttackBlockHandler*, RE::ButtonEvent*, void*);

    REL::Relocation<ProcessButtonFn> g_original{};

    // 最近一次攻击键按下时判定出的结果：耐力是否够发动强力攻击
    bool g_allowPowerAttack = true;

    int GamepadKeycode(std::uint32_t a_dx)
    {
        using Key = RE::BSWin32GamepadDevice::Key;
        switch (static_cast<Key>(a_dx)) {
            case Key::kUp:            return 266;
            case Key::kDown:          return 267;
            case Key::kLeft:          return 268;
            case Key::kRight:         return 269;
            case Key::kStart:         return 270;
            case Key::kBack:          return 271;
            case Key::kLeftThumb:     return 272;
            case Key::kRightThumb:    return 273;
            case Key::kLeftShoulder:  return 274;
            case Key::kRightShoulder: return 275;
            case Key::kA:             return 276;
            case Key::kB:             return 277;
            case Key::kX:             return 278;
            case Key::kY:             return 279;
            case Key::kLeftTrigger:   return 280;
            case Key::kRightTrigger:  return 281;
            default:                  return -1;
        }
    }

    bool IsAttackButton(const RE::ButtonEvent* a_event, bool& a_isLeft)
    {
        if (!a_event) {
            return false;
        }

        const auto& cfg    = Settings::Get();
        const auto  device = a_event->device.get();
        const auto  code   = a_event->GetIDCode();

        if (device == RE::INPUT_DEVICE::kMouse) {
            // 0 = 鼠标左键, 1 = 鼠标右键
            if (code != 0 && code != 1) {
                return false;
            }
            a_isLeft = (code == (cfg.reverseMouse ? 0u : 1u));
            return true;
        }

        if (device == RE::INPUT_DEVICE::kGamepad) {
            const int mapped = GamepadKeycode(code);
            if (mapped == cfg.leftButton || mapped == cfg.rightButton) {
                a_isLeft = (mapped == cfg.leftButton);
                return true;
            }
        }

        return false;
    }

    bool HasEnoughStaminaForPowerAttack()
    {
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return true;
        }

        auto weapon = static_cast<RE::TESObjectWEAP*>(player->GetEquippedObject(false));  // 右手
        if (!weapon || !weapon->IsWeapon()) {
            weapon = static_cast<RE::TESObjectWEAP*>(player->GetEquippedObject(true));    // 左手
        }

        const float cost    = Cost::PowerAttack(player, weapon);
        const float current = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

        if (Settings::Get().debugLog) {
            logger::info("power attack cost {:.2f} | current stamina {:.2f} | allowed {}",
                         cost, current, current >= cost);
        }

        return current >= cost;
    }

    void ProcessButton(RE::AttackBlockHandler* a_this, RE::ButtonEvent* a_event, void* a_data)
    {
        bool isLeft = false;

        if (Settings::Get().enabled && IsAttackButton(a_event, isLeft)) {
            if (a_event->IsDown()) {
                // 按下的瞬间做一次判定，本次按键期间都沿用这个结果
                g_allowPowerAttack = HasEnoughStaminaForPowerAttack();
                if (!g_allowPowerAttack) {
                    // 表现和原版耐力耗尽一致：耐力条闪一下
                    RE::FlashHUDMeter(RE::ActorValue::kStamina);
                }
                g_original(a_this, a_event, a_data);
                return;
            }

            // 耐力不够：把持续的 held 事件吞掉。
            // 引擎收不到按住的持续输入，就不会把这次攻击升级成强力攻击，
            // 玩家看到的仍然是普通攻击，手感和原版一致。
            if (a_event->IsHeld() && !g_allowPowerAttack) {
                return;
            }
        }

        g_original(a_this, a_event, a_data);
    }
}

namespace Hook
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> vtable{ RE::VTABLE_AttackBlockHandler[0] };
        g_original = stl::unrestricted_cast<ProcessButtonFn>(
            vtable.write_vfunc(4, reinterpret_cast<std::uintptr_t>(&ProcessButton)));
        logger::info("AttackBlockHandler::ProcessButton hooked");
    }
}
