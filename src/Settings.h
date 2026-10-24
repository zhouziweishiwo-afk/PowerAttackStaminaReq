#pragma once

namespace Settings
{
    struct Config
    {
        bool enabled{ true };
        bool reverseMouse{ false };
        int  leftButton{ 280 };   // 手柄左扳机
        int  rightButton{ 281 };  // 手柄右扳机
        bool debugLog{ false };
    };

    const Config& Get() noexcept;
    void          Load();
}
