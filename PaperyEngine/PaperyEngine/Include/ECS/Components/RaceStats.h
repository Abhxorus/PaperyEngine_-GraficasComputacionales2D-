#pragma once

namespace ECS {
    struct RaceStats {
        int currentLap{ 1 };
        int lastCheckpointPassed{ -1 }; // -1 significa que no ha pasado el 0 aún

        bool hasFinished{ false };
        int finalPosition{ 0 }; // 1er lugar, 2do lugar, etc.
    };
}