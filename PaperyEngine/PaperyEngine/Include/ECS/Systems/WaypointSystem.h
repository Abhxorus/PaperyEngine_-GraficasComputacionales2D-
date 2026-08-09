#pragma once
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/WaypointPath.h"
#include "ECS/Components/PathFollower.h"
#include <cmath>
#include <cstdlib>

namespace ECS {

    class WaypointSystem final : public System {
    public:
        WaypointSystem() = default;

        void OnUpdate(Registry& registry, float /*deltaTime*/) override {

            // Iteramos sobre todos los karts que tienen un "GPS" (PathFollower)
            registry.GetView<Transform, SteeringBehavior, PathFollower>().Each(
                [&registry](EntityID, Transform& t, SteeringBehavior& behavior, PathFollower& follower) {

                    // Verificamos que la pista maestra exista
                    if (!registry.IsAlive(follower.pathEntity) || !registry.HasComponent<WaypointPath>(follower.pathEntity)) return;

                    auto& path = registry.GetComponent<WaypointPath>(follower.pathEntity);
                    if (path.points.empty()) return;

                    // 1. Calcular el objetivo real (Centro del nodo + Margen del carril)
                    sf::Vector2f baseTarget = path.points[follower.currentIndex];
                    sf::Vector2f actualTarget = baseTarget + follower.currentOffset;

                    // 2. Distancia al objetivo
                    sf::Vector2f toTarget = actualTarget - t.position;
                    float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

                    // 3. Si llegamos, pasamos al siguiente nodo y generamos un nuevo margen aleatorio
                    if (dist < path.reachRadius) {
                        follower.currentIndex++;

                        if (follower.currentIndex >= path.points.size()) {
                            if (path.isLoop) {
                                follower.currentIndex = 0;
                            }
                            else {
                                follower.currentIndex = path.points.size() - 1;
                                behavior.seekEnabled = false;
                                behavior.arriveEnabled = true;
                            }
                        }

                        // Generar un nuevo offset para que el kart no vaya por el centro exacto
                        float randX = ((static_cast<float>(rand()) / RAND_MAX) * 2.f - 1.f) * follower.laneOffsetMax;
                        float randY = ((static_cast<float>(rand()) / RAND_MAX) * 2.f - 1.f) * follower.laneOffsetMax;
                        follower.currentOffset = { randX, randY };
                    }

                    // 4. Inyectar el nuevo objetivo al comportamiento físico
                    behavior.target = path.points[follower.currentIndex] + follower.currentOffset;

                    if (path.isLoop || follower.currentIndex < path.points.size() - 1) {
                        behavior.seekEnabled = true;
                        behavior.arriveEnabled = false;
                    }
                });
        }
    };

} // namespace ECS