#include "PCH.h"
#include "Cost.h"

namespace
{
    float GameSettingFloat(const char* a_name, float a_default)
    {
        const auto collection = RE::GameSettingCollection::GetSingleton();
        if (!collection) {
            return a_default;
        }
        const auto setting = collection->GetSetting(a_name);
        if (!setting) {
            return a_default;
        }
        return setting->GetFloat();
    }
}

namespace Cost
{
    float PowerAttack(RE::Actor* a_actor, RE::TESObjectWEAP* a_weapon)
    {
        if (!a_actor) {
            return 0.0f;
        }

        // 原版公式： (base + 武器重量 * mult) * 强力攻击惩罚系数
        // 默认 GMST: base = 20, mult = 1, penalty = 2  =>  (20 + weight) * 2
        const float base    = GameSettingFloat("fStaminaAttackWeaponBase", 20.0f);
        const float mult    = GameSettingFloat("fStaminaAttackWeaponMult", 1.0f);
        const float penalty = GameSettingFloat("fPowerAttackStaminaPenalty", 2.0f);

        const float weight = a_weapon ? a_weapon->weight : 0.0f;
        float       cost   = (base + weight * mult) * penalty;

        // Perk Entry Point 27 = Mod Power Attack Stamina。
        // 所有 perk（如减少强力攻击消耗）、武器附魔、以及当时生效的药水/法术效果，
        // 都会在这里被逐个应用到 cost 上，得到最终的实际消耗。
        RE::BGSEntryPoint::HandleEntryPoint(
            static_cast<RE::BGSEntryPoint::ENTRY_POINT>(27),
            a_actor,
            a_weapon,
            &cost);

        if (cost < 0.0f) {
            cost = 0.0f;
        }
        return cost;
    }
}
