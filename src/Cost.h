#pragma once

namespace Cost
{
    // 本次强力攻击真正会消耗的耐力值。
    // 已累计当时生效的 perk / 装备附魔 / 药水 的全部修正。
    float PowerAttack(RE::Actor* a_actor, RE::TESObjectWEAP* a_weapon);
}
