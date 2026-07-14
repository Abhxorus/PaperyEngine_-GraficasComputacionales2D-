/**
 * @file SteeringAgent.h
 * @brief Definición del componente SteeringAgent para el Entity Component System (ECS).
 */

#pragma once
#include "ECS/Types.h"
#include <SFML/System/Vector2.hpp>

namespace ECS {
    /**
     * @struct SteeringAgent
     * @brief Componente de datos que define las propiedades físicas de un agente autónomo.
     * @details Almacena el estado cinemático (velocidad, aceleración) y los límites de
     * movimiento (fuerza, velocidad y masa) requeridos por el SteeringSystem para la
     * integración de Euler y la simulación de comportamientos de dirección.
     */
    struct SteeringAgent {
        /**
         * @brief Vector de velocidad actual del agente en el espacio 2D.
         */
        sf::Vector2f velocity{ 0.f, 0.f };

        /**
         * @brief Vector de aceleración actual, calculado a partir de las fuerzas aplicadas en el frame.
         */
        sf::Vector2f acceleration{ 0.f, 0.f };

        /**
         * @brief Magnitud máxima a la que el agente puede desplazarse.
         */
        float maxSpeed{ 150.f };

        /**
         * @brief Fuerza máxima de dirección aplicable, lo que limita la capacidad de giro del agente.
         */
        float maxForce{ 50.f };

        /**
         * @brief Masa del agente, utilizada para determinar la aceleración final (a = F/m).
         */
        float mass{ 1.f };
    };
}