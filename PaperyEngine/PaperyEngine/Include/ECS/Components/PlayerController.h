#pragma once

namespace ECS {
    struct PlayerController {
        float turnSpeed{ 150.f };
        float acceleration{ 600.f };
        float currentSpeed{ 0.f }; // Velocidad actual (escalar)
    };
}