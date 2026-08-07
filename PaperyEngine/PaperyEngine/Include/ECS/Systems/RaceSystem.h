#pragma once
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Checkpoint.h"
#include "ECS/Components/RaceStats.h"
#include "ECS/Components/PlayerController.h"
#include "ECS/Components/SteeringAgent.h"
#include <cmath>
#include <iostream>
#include <string>

namespace ECS {

    class RaceSystem final : public System {
    public:
        RaceSystem() = default;

        void OnUpdate(Registry& registry, float /*deltaTime*/) override {

            // Cuántos checkpoints hay en total en el mapa
            int totalCheckpoints = 0;
            registry.GetView<Checkpoint>().Each([&](EntityID, Checkpoint&) {
                totalCheckpoints++;
                });

            if (totalCheckpoints == 0) return;

            // Revisamos cada kart que esté compitiendo
            registry.GetView<Transform, RaceStats>().Each(
                [&](EntityID kartId, Transform& kartTransform, RaceStats& stats) {

                    if (stats.hasFinished) return; // Si ya terminó, lo ignoramos

                    // Comparamos el kart contra todos los checkpoints
                    registry.GetView<Transform, Checkpoint>().Each(
                        [&](EntityID cpId, Transform& cpTransform, Checkpoint& cp) {

                            // Distancia entre kart y checkpoint
                            sf::Vector2f diff = kartTransform.position - cpTransform.position;
                            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                            if (dist < cp.radius) {
                                // Lógica de progresión estricta: debe pasar por el siguiente en orden
                                if (cp.index == stats.lastCheckpointPassed + 1) {
                                    stats.lastCheckpointPassed = cp.index;

                                    // Si era la línea de meta (y es el último checkpoint)
                                    if (cp.isFinishLine && cp.index == totalCheckpoints - 1) {
                                        stats.currentLap++;
                                        stats.lastCheckpointPassed = -1; // Reiniciamos para la siguiente vuelta

                                        if (stats.currentLap > 3) {
                                            stats.hasFinished = true;
                                            m_currentPlacement++;
                                            stats.finalPosition = m_currentPlacement;

                                            // Detener el coche físicamente si es la IA
                                            if (auto* agent = registry.TryGetComponent<SteeringAgent>(kartId)) {
                                                agent->velocity = { 0.f, 0.f };
                                            }

                                            std::cout << "Kart " << kartId << " termino en la posicion: " << stats.finalPosition << "!\n";
                                        }
                                    }
                                }
                            }
                        });
                });
        }

    private:
        int m_currentPlacement{ 0 }; // Contador global de posiciones en la carrera
    };

} // namespace ECS