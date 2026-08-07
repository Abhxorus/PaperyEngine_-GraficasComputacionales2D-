#pragma once
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/SteeringAgent.h"
#include "ECS/Components/PlayerController.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

namespace ECS {

    class PlayerInputSystem final : public System {
    public:
        PlayerInputSystem() = default;

        void OnUpdate(Registry& registry, float deltaTime) override {
            registry.GetView<Transform, SteeringAgent, PlayerController>().Each(
                [deltaTime](EntityID, Transform& t, SteeringAgent& agent, PlayerController& player) {

                    // 1. Giro libre
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                        t.rotation -= player.turnSpeed * deltaTime;
                    }
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                        t.rotation += player.turnSpeed * deltaTime;
                    }

                    // 2. Aceleración tipo Arcade (Magnitud pura)
                    bool isMoving = false;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                        player.currentSpeed += player.acceleration * deltaTime;
                        isMoving = true;
                    }
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                        player.currentSpeed -= player.acceleration * deltaTime;
                        isMoving = true;
                    }

                    // Fricción constante al soltar el acelerador (freno motor)
                    if (!isMoving) {
                        if (player.currentSpeed > 0) {
                            player.currentSpeed -= player.acceleration * 0.8f * deltaTime;
                            if (player.currentSpeed < 0) player.currentSpeed = 0;
                        }
                        else if (player.currentSpeed < 0) {
                            player.currentSpeed += player.acceleration * 0.8f * deltaTime;
                            if (player.currentSpeed > 0) player.currentSpeed = 0;
                        }
                    }

                    // Topes de velocidad vinculados a las capacidades del agente
                    if (player.currentSpeed > agent.maxSpeed) player.currentSpeed = agent.maxSpeed;
                    if (player.currentSpeed < -agent.maxSpeed / 2.f) player.currentSpeed = -agent.maxSpeed / 2.f; // Reversa más lenta

                    // 3. Convertir rotación a vector direccional
                    // Restamos 90 grados matemáticos para que 0 grados signifique "hacia arriba" en la pantalla de SFML
                    float rotRadians = (t.rotation - 90.f) * 3.14159265f / 180.f;
                    sf::Vector2f forwardDirection(std::cos(rotRadians), std::sin(rotRadians));

                    // 4. Aplicar movimiento estricto hacia adelante (elimina el derrape de hielo)
                    agent.velocity = forwardDirection * player.currentSpeed;
                    t.position += agent.velocity * deltaTime;
                });
        }
    };

} // namespace ECS