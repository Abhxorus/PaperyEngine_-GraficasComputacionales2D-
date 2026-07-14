/**
 * @file SteeringBehavior.h
 * @brief Definición del componente SteeringBehavior para el Entity Component System (ECS).
 */

#pragma once
#include "ECS/Types.h"
#include <SFML/System/Vector2.hpp>

namespace ECS {
    /**
     * @struct SteeringBehavior
     * @brief Componente de datos que almacena las intenciones y parámetros de dirección de un agente.
     * @details Controla qué comportamientos de dirección (Steering Behaviors) están activos y define
     * los parámetros específicos necesarios para calcular cada fuerza (como radios, distancias y objetivos).
     */
    struct SteeringBehavior {
        /**
         * @brief Posición objetivo en el espacio 2D. Utilizada principalmente por Seek, Flee y Arrive.
         */
        sf::Vector2f target{ 0.f, 0.f };

        // Banderas de activación
        /** @brief Activa o desactiva el comportamiento de Seek (búsqueda directa hacia el objetivo). */
        bool seekEnabled{ false };
        /** @brief Activa o desactiva el comportamiento de Flee (huida directa desde el objetivo). */
        bool fleeEnabled{ false };
        /** @brief Activa o desactiva el comportamiento de Arrive (llegada suave al objetivo). */
        bool arriveEnabled{ false };
        /** @brief Activa o desactiva el comportamiento de Wander (merodeo aleatorio). */
        bool wanderEnabled{ false };
        /** @brief Activa o desactiva el comportamiento de Pursuit (persecución predictiva de una entidad). */
        bool pursuitEnabled{ false };
        /** @brief Activa o desactiva el comportamiento de Obstacle Avoidance (evasión de obstáculos). */
        bool avoidanceEnabled{ false };

        // Parámetros
        /**
         * @brief Radio de frenado para el comportamiento Arrive.
         * Cuando la distancia al objetivo es menor a este radio, el agente comienza a desacelerar.
         */
        float slowingRadius{ 100.f };    // Para Arrive

        /** @brief Radio del círculo imaginario utilizado para calcular el desplazamiento en Wander. */
        float wanderRadius{ 50.f };      // Para Wander
        /** @brief Distancia a la que se proyecta el círculo de Wander frente al agente. */
        float wanderDistance{ 100.f };
        /** @brief Magnitud máxima de la variación aleatoria aplicada al ángulo de Wander en cada frame. */
        float wanderJitter{ 20.f };
        /** @brief Ángulo actual en el círculo de Wander (estado interno modificado frame a frame). */
        float wanderAngle{ 0.f };

        /**
         * @brief Identificador de la entidad objetivo a interceptar.
         * Utilizado por el comportamiento Pursuit para predecir la posición futura.
         */
        EntityID pursuitTarget{ NULL_ENTITY }; // Para Pursuit

        /**
         * @brief Longitud de la antena o sensor (feeler) de visión frontal.
         * Utilizado en Obstacle Avoidance para detectar colisiones inminentes.
         */
        float avoidanceFeelerLength{ 100.f };  // Para Obstacle Avoidance
    };
}