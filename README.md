# Power Attack Stamina Requirement

Skyrim SE / AE（1.7.104）的 SKSE 插件。

角色当前耐力 **低于本次强力攻击的真实消耗**（已累计当时生效的 perk、装备附魔、药水的全部修正）时，
无法发动强力攻击，只能打出普通攻击，并像原版耐力耗尽那样闪一下耐力条。

## 和原版的区别

原版只要求「耐力 ≥ 1 点」就能打出强力攻击 —— 只要耐力药还在持续回耐，就能无限强力攻击。
本插件把门槛改成「耐力 ≥ 这次强力攻击实际要花掉的耐力」。

## 消耗值怎么算

```
( fStaminaAttackWeaponBase + 武器重量 * fStaminaAttackWeaponMult ) * fPowerAttackStaminaPenalty
```

默认 GMST 是 base = 20、mult = 1、penalty = 2，也就是 `(20 + 武器重量) * 2 = 40 + 2 * 武器重量`。

算完基础值之后，再把这个值交给引擎的 **Perk Entry Point 27（Mod Power Attack Stamina）**。
perk（例如减少强力攻击消耗的那些）、武器附魔、以及**当时正在生效的药水 / 法术**，
对强力攻击消耗的所有修正都会在这里被逐个累计进去。

所以喝一瓶减耗药水的瞬间，门槛会立刻跟着变低 —— 判定的始终是「当时的最终消耗值」。

## 实现原理

hook `AttackBlockHandler::ProcessButton`（虚表第 4 项）。攻击键**按下的瞬间**算一次消耗并与当前耐力比较：

- **够** → 原样放行，行为和原版完全一致。
- **不够** → 闪一下耐力条，然后把后续的 `held`（持续按住）事件吞掉。引擎收不到持续按住的输入，
  就不会把这次攻击升级成强力攻击。

只在按下时判定一次，**不接管攻击的派发**。因此不会改攻击手感、不影响普通攻击、
也不和 MCO / AMR 这类动画 mod 抢输入控制权。

## 安装

把 `PowerAttackStaminaReq.dll` 放到 MO2 里：

```
<MO2 mods 目录>\PowerAttackStaminaReq\SKSE\Plugins\PowerAttackStaminaReq.dll
```

然后启用这个 mod。前提（一般都已经有了）：

- SKSE64 2.3.1（对应 1.7.104）
- Address Library for SKSE Plugins v13

## 配置

可选。放在 `Data/SKSE/Plugins/PowerAttackStaminaReq.ini`：

| 键 | 默认 | 说明 |
|---|---|---|
| `Enabled` | 1 | 0 关闭插件逻辑 |
| `DebugLog` | 0 | 1 时把每次判定的消耗值和当前耐力写进日志 |
| `ReverseMouseButtons` | 0 | 游戏里交换过鼠标左右键时设 1 |
| `LeftButton` | 280 | 手柄左手攻击键 |
| `RightButton` | 281 | 手柄右手攻击键 |

## 编译

仓库里的 GitHub Actions 会自动编译：push 到 `main` 即触发，
产物（dll + pdb）在 Actions 页面的 Artifacts 里下载。

本地编译（VS2022 + vcpkg + CMake）：

```bat
cmake --preset vs-release
cmake --build build --config Release
```

设置 `SKYRIM_MODS_FOLDER` 环境变量后，编译完成会自动把 dll 拷进 MO2 的 mods 目录。

## 已知限制

- **只作用于玩家**，NPC 的强力攻击不检查。
- 只识别**鼠标和手柄**的攻击键。如果把攻击绑到了键盘按键上，当前版本不会拦截（也不会有副作用）。
- 一次按键只在按下瞬间判定一次：按住期间即便耐力回复了也不会中途解锁，松开重按即可。
- 双持强力攻击按主手武器重量计算。
- 空手时武器重量按 0 算，消耗 = `base * penalty` = 40。
