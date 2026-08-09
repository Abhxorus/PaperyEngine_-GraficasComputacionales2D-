#pragma once
#include "ECS/Types.h"
#include <SFML/System/Vector2.hpp>

namespace ECS {
    struct PathFollower {
        EntityID pathEntity{ NULL_ENTITY }; // ID de la entidad que tiene la ruta maestra
        std::size_t currentIndex{ 0 };      // Por qué nodo va ESTE kart específico

        float laneOffsetMax{ 50.f };        // Qué tanto se puede desviar del centro del punto
        sf::Vector2f currentOffset{ 0.f, 0.f }; // Offset actual calculado
    };
}