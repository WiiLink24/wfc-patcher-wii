#pragma once

#include <wwfcCommon.h>

namespace wwfc::EGG
{

class SceneManager
{
public:
    int getCurrentSceneID() const
    {
        return m_currentSceneID;
    }

private:
    /* 0x00 */ u8 _00[0x18 - 0x00];
    /* 0x18 */ int m_currentSceneID;
    /* 0x1C */ u8 _1C[0x30 - 0x1C];
};

static_assert(sizeof(SceneManager) == 0x30);

} // namespace wwfc::EGG
