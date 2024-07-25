#pragma once

#include <wwfcUtil.h>

namespace mkw::System
{

#if RMC

class GameScene
{
public:
    bool involuntarilySkippedFrame()
    {
        return m_involuntarilySkippedFrame;
    }

    static GameScene* Instance()
    {
        LONGCALL GameScene* Instance()
            AT(RMCXD_PORT(0x8051BED0, 0x80517A5C, 0x8051B850, 0x80509EF0));

        return Instance();
    }

private:
    /* 0x0000 */ u8 _0000[0x2538 - 0x0000];
    /* 0x2538 */ bool m_involuntarilySkippedFrame;
    /* 0x2539 */ u8 _2539[0x254C - 0x2539];
};

static_assert(sizeof(GameScene) == 0x254C);

#endif

} // namespace mkw::System
