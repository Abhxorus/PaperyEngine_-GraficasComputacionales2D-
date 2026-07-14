/**
 * @file Obstacle.h
 * @brief Definición del componente Obstacle para el Entity Component System (ECS).
 */

#pragma once

namespace ECS {
    /**
     * @struct Obstacle
     * @brief Componente de datos que define a una entidad como un obstáculo evitable.
     * @details Este componente es utilizado por el comportamiento de "Obstacle Avoidance"
     * en el SteeringSystem para calcular distancias de intersección y generar fuerzas de repulsión.
     */
    struct Obstacle {
        /**
         * @brief Radio de colisión circular del obstáculo.
         * Define el área de influencia alrededor del centro de la entidad que los
         * agentes intentarán esquivar.
         */
        float radius{ 40.f }; // Radio de colisión para ser esquivado
    };
}