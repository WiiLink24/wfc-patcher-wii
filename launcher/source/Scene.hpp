#pragma once

#include <ogc/gx.h>
#include "Util.hpp"

class Scene
{
public:
    enum class ShutdownType {
        NONE,
        POWER_OFF,
        EXIT,
        LAUNCH,
    };

    static void Init(GXRModeObj* rmode);

    static void StartThread();
    static void ShutdownAsync(ShutdownType type);
    static void Shutdown(ShutdownType type);

    static Rect GetProjectionRect();
};