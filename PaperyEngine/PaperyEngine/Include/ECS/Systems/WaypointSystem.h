#pragma once
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/WaypointPath.h"
#include <cmath>

namespace ECS {

    class WaypointSystem final : public System {
    public:
        WaypointSystem() = default;

        void OnUpdate(Registry& registry, float /*deltaTime*/) override {
            registry.GetView<Transform, SteeringBehavior, WaypointPath>().Each(
                [](EntityID, Transform& t, SteeringBehavior& behavior, WaypointPath& path) {
                    if (path.points.empty()) return;

                    // 1. Obtener la posición del objetivo actual
                    sf::Vector2f currentTarget = path.points[path.currentIndex];
                    sf::Vector2f toTarget = currentTarget - t.position;

                    // 2. Calcular la distancia al objetivo
                    float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

                    // 3. Si llegamos al punto, pasamos al siguiente
                    if (dist < path.reachRadius) {
                        path.currentIndex++;

                        // Validar si completamos la ruta
                        if (path.currentIndex >= path.points.size()) {
                            if (path.isLoop) {
                                path.currentIndex = 0; // Vuelta nueva
                            }
                            else {
                                path.currentIndex = path.points.size() - 1;
                                behavior.seekEnabled = false;
                                behavior.arriveEnabled = true; // Frena suavemente al terminar
                            }
                        }
                    }

                    // 4. Inyectar el nuevo objetivo al comportamiento físico
                    behavior.target = path.points[path.currentIndex];

                    // Mantener a la IA buscando el punto activo
                    if (path.isLoop || path.currentIndex < path.points.size() - 1) {
                        behavior.seekEnabled = true;
                        behavior.arriveEnabled = false;
                    }
                });
        }
    };

} // namespace ECS