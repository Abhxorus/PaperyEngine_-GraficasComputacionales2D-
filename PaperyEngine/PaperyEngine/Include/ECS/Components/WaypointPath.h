#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>

namespace ECS {
    struct WaypointPath {
        std::vector<sf::Vector2f> points;
        std::size_t currentIndex{ 0 };

        // Distancia a la que consideramos que el kart "tocó" el punto y debe ir al siguiente
        float reachRadius{ 60.f };

        // Si es verdadero, al llegar al final reinicia el circuito (ideal para las 3 vueltas)
        bool isLoop{ true };
    };
}