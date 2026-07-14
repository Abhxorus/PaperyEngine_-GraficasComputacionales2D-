/**
 * @file SteeringSystem.h
 * @brief Implementación del sistema de Steering Behaviors para el Entity Component System (ECS).
 * @details Este sistema se encarga de calcular y aplicar las fuerzas de dirección
 * (Seek, Flee, Arrive, Wander, Pursuit, Obstacle Avoidance) a las entidades que poseen
 * los componentes Transform, SteeringAgent y SteeringBehavior.
 */

#pragma once
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/SteeringAgent.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/Obstacle.h"
#include <cmath>
#include <cstdlib>

namespace ECS {

    /**
     * @brief Calcula la magnitud (longitud) de un vector 2D.
     * @param v El vector a evaluar.
     * @return La longitud del vector.
     */
    inline float VectorLength(const sf::Vector2f& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    /**
     * @brief Normaliza un vector 2D (lo convierte en un vector unitario de longitud 1).
     * @param v El vector a normalizar.
     * @return El vector normalizado, o un vector (0,0) si la longitud es cercana a cero.
     */
    inline sf::Vector2f VectorNormalize(const sf::Vector2f& v) {
        float len = VectorLength(v);
        return (len > 0.0001f) ? sf::Vector2f(v.x / len, v.y / len) : sf::Vector2f(0.f, 0.f);
    }

    /**
     * @brief Trunca un vector para que su magnitud no exceda un valor máximo.
     * @param v El vector original a truncar.
     * @param max La magnitud máxima permitida.
     * @return El vector truncado si excede el máximo permitido, de lo contrario devuelve el mismo vector.
     */
    inline sf::Vector2f VectorTruncate(const sf::Vector2f& v, float max) {
        float len = VectorLength(v);
        if (len > max) {
            return VectorNormalize(v) * max;
        }
        return v;
    }

    /**
     * @class SteeringSystem
     * @brief Sistema responsable de actualizar el movimiento autónomo de los agentes en el entorno.
     * @details Itera sobre todas las entidades con SteeringAgent y SteeringBehavior,
     * calculando las fuerzas resultantes basadas en los comportamientos activos, y aplicando
     * una integración de Euler básica para modificar su componente Transform en cada frame.
     */
    class SteeringSystem final : public System {
    public:
        /**
         * @brief Constructor por defecto.
         */
        SteeringSystem() = default;

        /**
         * @brief Método de actualización llamado en cada frame para procesar el movimiento.
         * @param registry Referencia al registro del ECS para acceder a entidades y sus componentes.
         * @param deltaTime Tiempo transcurrido desde el último frame (para cálculos de físicas independientes del framerate).
         */
        void OnUpdate(Registry& registry, float deltaTime) override {
            registry.GetView<Transform, SteeringAgent, SteeringBehavior>().Each(
                [this, &registry, deltaTime](EntityID entity, Transform& t, SteeringAgent& agent, SteeringBehavior& behavior) {

                    sf::Vector2f steeringForce{ 0.f, 0.f };

                    if (behavior.seekEnabled)   steeringForce += Seek(t.position, agent, behavior.target);
                    if (behavior.fleeEnabled)   steeringForce += Flee(t.position, agent, behavior.target);
                    if (behavior.arriveEnabled) steeringForce += Arrive(t.position, agent, behavior.target, behavior.slowingRadius);
                    if (behavior.wanderEnabled) steeringForce += Wander(agent, behavior);

                    if (behavior.pursuitEnabled && behavior.pursuitTarget != NULL_ENTITY && registry.IsAlive(behavior.pursuitTarget)) {
                        if (auto* targetT = registry.TryGetComponent<Transform>(behavior.pursuitTarget)) {
                            sf::Vector2f targetVel{ 0.f, 0.f };
                            if (auto* targetAgent = registry.TryGetComponent<SteeringAgent>(behavior.pursuitTarget)) {
                                targetVel = targetAgent->velocity;
                            }
                            steeringForce += Pursuit(t.position, agent, targetT->position, targetVel);
                        }
                    }

                    if (behavior.avoidanceEnabled) {
                        steeringForce += AvoidObstacles(t.position, agent, behavior, registry, entity);
                    }

                    // Físicas básicas (Euler)
                    steeringForce = VectorTruncate(steeringForce, agent.maxForce);
                    agent.acceleration = steeringForce / agent.mass;

                    agent.velocity += agent.acceleration * deltaTime;
                    agent.velocity = VectorTruncate(agent.velocity, agent.maxSpeed);
                    t.position += agent.velocity * deltaTime;

                    // Rotación orientada a la velocidad
                    if (VectorLength(agent.velocity) > 0.1f) {
                        t.rotation = std::atan2(agent.velocity.y, agent.velocity.x) * 180.f / 3.14159265f;
                    }

                    agent.acceleration = { 0.f, 0.f };
                });
        }

    private:
        /**
         * @brief Calcula la fuerza necesaria para dirigirse en línea recta hacia una posición.
         * @param pos Posición actual del agente.
         * @param agent Componente con las propiedades físicas (velocidad máxima).
         * @param target Coordenada del objetivo.
         * @return Vector de fuerza de búsqueda (Seek).
         */
        sf::Vector2f Seek(const sf::Vector2f& pos, const SteeringAgent& agent, const sf::Vector2f& target) {
            sf::Vector2f desired = target - pos;
            desired = VectorNormalize(desired) * agent.maxSpeed;
            return desired - agent.velocity;
        }

        /**
         * @brief Calcula la fuerza necesaria para alejarse en línea recta desde una posición.
         * @param pos Posición actual del agente.
         * @param agent Componente con las propiedades físicas (velocidad máxima).
         * @param target Coordenada a evadir.
         * @return Vector de fuerza de huida (Flee).
         */
        sf::Vector2f Flee(const sf::Vector2f& pos, const SteeringAgent& agent, const sf::Vector2f& target) {
            sf::Vector2f desired = pos - target;
            desired = VectorNormalize(desired) * agent.maxSpeed;
            return desired - agent.velocity;
        }

        /**
         * @brief Calcula la fuerza para acercarse a un objetivo y frenar suavemente al estar cerca.
         * @param pos Posición actual del agente.
         * @param agent Componente con las propiedades físicas (velocidad máxima).
         * @param target Coordenada del objetivo.
         * @param radius Distancia dentro de la cual el agente comenzará a desacelerar progresivamente.
         * @return Vector de fuerza de llegada (Arrive).
         */
        sf::Vector2f Arrive(const sf::Vector2f& pos, const SteeringAgent& agent, const sf::Vector2f& target, float radius) {
            sf::Vector2f desired = target - pos;
            float dist = VectorLength(desired);
            if (dist == 0.f) return { 0.f, 0.f };

            desired = VectorNormalize(desired);
            if (dist < radius) {
                desired *= agent.maxSpeed * (dist / radius);
            }
            else {
                desired *= agent.maxSpeed;
            }
            return desired - agent.velocity;
        }

        /**
         * @brief Produce una fuerza que genera un patrón de movimiento aleatorio y fluido (merodeo).
         * @param agent Componente con las propiedades físicas actuales.
         * @param behavior Componente de comportamiento que almacena el estado (ángulo y radios) de Wander.
         * @return Vector de fuerza de merodeo (Wander).
         */
        sf::Vector2f Wander(const SteeringAgent& agent, SteeringBehavior& behavior) {
            float randomX = ((static_cast<float>(rand()) / RAND_MAX) * 2.f) - 1.f;
            float randomY = ((static_cast<float>(rand()) / RAND_MAX) * 2.f) - 1.f;

            behavior.wanderAngle += (randomX * behavior.wanderJitter);

            sf::Vector2f agentDir = VectorNormalize(agent.velocity);
            if (VectorLength(agentDir) == 0.f) agentDir = { 1.f, 0.f };

            sf::Vector2f circleCenter = agentDir * behavior.wanderDistance;
            sf::Vector2f displacement = {
                std::cos(behavior.wanderAngle) * behavior.wanderRadius,
                std::sin(behavior.wanderAngle) * behavior.wanderRadius
            };

            return circleCenter + displacement;
        }

        /**
         * @brief Calcula la fuerza para interceptar a un objetivo móvil, prediciendo su posición futura.
         * @param pos Posición actual del agente perseguidor.
         * @param agent Componente con las propiedades físicas del perseguidor.
         * @param targetPos Posición actual del objetivo.
         * @param targetVel Velocidad actual del objetivo.
         * @return Vector de fuerza de persecución (Pursuit).
         */
        sf::Vector2f Pursuit(const sf::Vector2f& pos, const SteeringAgent& agent, const sf::Vector2f& targetPos, const sf::Vector2f& targetVel) {
            sf::Vector2f toEvader = targetPos - pos;
            float relativeHeading = (VectorNormalize(agent.velocity).x * VectorNormalize(targetVel).x) +
                (VectorNormalize(agent.velocity).y * VectorNormalize(targetVel).y);

            if (relativeHeading < -0.95f) {
                return Seek(pos, agent, targetPos);
            }

            float lookAheadTime = VectorLength(toEvader) / (agent.maxSpeed + VectorLength(targetVel));
            sf::Vector2f futureTarget = targetPos + (targetVel * lookAheadTime);
            return Seek(pos, agent, futureTarget);
        }

        /**
         * @brief Detecta y genera fuerzas para esquivar entidades que poseen el componente Obstacle.
         * @param pos Posición actual del agente.
         * @param agent Componente físico del agente.
         * @param behavior Componente con los parámetros del feeler (sensor de distancia frontal).
         * @param registry Referencia al registro del ECS para iterar sobre los obstáculos.
         * @param self ID de la entidad evaluada para evitar auto-evasión.
         * @return Vector de fuerza de repulsión lateral para evitar la colisión.
         */
        sf::Vector2f AvoidObstacles(const sf::Vector2f& pos, const SteeringAgent& agent, const SteeringBehavior& behavior, Registry& registry, EntityID self) {
            sf::Vector2f avoidanceForce{ 0.f, 0.f };
            float dynamicFeeler = behavior.avoidanceFeelerLength * (VectorLength(agent.velocity) / agent.maxSpeed);
            if (dynamicFeeler < behavior.avoidanceFeelerLength * 0.5f) dynamicFeeler = behavior.avoidanceFeelerLength * 0.5f;

            registry.GetView<Transform, Obstacle>().Each(
                [&](EntityID obsEntity, Transform& obsTransform, Obstacle& obstacle) {
                    if (obsEntity == self) return; // No evitarse a sí mismo

                    sf::Vector2f toObstacle = obsTransform.position - pos;
                    float dist = VectorLength(toObstacle);

                    // Si está dentro de la longitud de visión (feeler)
                    if (dist > 0.f && dist < dynamicFeeler + obstacle.radius) {
                        sf::Vector2f agentDir = VectorNormalize(agent.velocity);

                        // Validar si el obstáculo está "al frente" del agente
                        float dotProd = (agentDir.x * toObstacle.x) + (agentDir.y * toObstacle.y);
                        if (dotProd > 0.f) {
                            // Fuerza de repulsión lateral
                            sf::Vector2f repel = pos - obsTransform.position;
                            avoidanceForce += VectorNormalize(repel) * agent.maxSpeed * (1.f - (dist / dynamicFeeler));
                        }
                    }
                });

            return avoidanceForce;
        }
    };
}