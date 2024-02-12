#pragma once

class Apploader
{
public:
    enum class State {
        INITIALIZING,
        DISC_SPINUP,
        READING_DISC,
        LAUNCHING,

        NO_DISC,
        READ_ERROR,
        UNSUPPORTED_GAME,
        FATAL_ERROR,
    };

    static State GetState();

    static void StartThread();
    static void Shutdown();
};