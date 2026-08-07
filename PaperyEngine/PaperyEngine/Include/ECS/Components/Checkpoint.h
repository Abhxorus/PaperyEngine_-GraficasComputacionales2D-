#pragma once

namespace ECS {
    struct Checkpoint {
        int index{ 0 };       // Orden del checkpoint (0, 1, 2, 3...)
        float radius{ 80.f }; // Qué tan ancha es la zona para detectarlo
        bool isFinishLine{ false }; // ¿Es la línea de meta?
    };
}