#include "Prerequisites.h"
#include "Core/Window.h"
#include "Core/CShape.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/SteeringAgent.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/Obstacle.h"
#include "ECS/Components/PlayerController.h" 
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/UISystem.h"
#include "ECS/Systems/SteeringSystem.h"
#include "ECS/Systems/PlayerInputSystem.h"   
#include "ECS/Components/WaypointPath.h"
#include "ECS/Systems/WaypointSystem.h"
#include "ECS/Systems/RaceSystem.h"
#include "ECS/Components/RaceStats.h"
#include "ECS/Components/Checkpoint.h"

/**
 * @file main.cpp
 * @brief Punto de entrada principal de la aplicación y ciclo de ejecución del motor.
 */

 /** @brief Instancia global de la ventana de la aplicación. */
Window g_window(Window(800, 600, "Papery Engine"));
ECS::Registry registry;

/** @brief Instancia de prueba para una figura geométrica de tipo círculo. */
CShape Circle(ShapeType::CIRCLE);

/** @brief Instancia de prueba para una figura geométrica de tipo línea. */
CShape line(ShapeType::LINE);

/**
 * @brief Libera los recursos globales o realiza tareas de limpieza antes de cerrar la aplicación.
 */
void destroy()
{
    ImGui::SFML::Shutdown();
}

/**
 * @brief Función principal del programa.
 * @details Inicializa la ventana y los objetos, y mantiene activo el bucle principal (Game Loop)
 * donde se procesan eventos, se actualiza la lógica y se renderizan los gráficos en pantalla.
 * @return Código de salida del programa (0 indica ejecución exitosa).
 */
int main()
{
    // m_window es un puntero a sf::RenderWindow.
    if (!ImGui::SFML::Init(*g_window.m_window)) {
        return -1;
    }

    // 1. Guardamos las referencias de los sistemas lógicos al registrarlos
    auto& inputSys = registry.AddSystem<ECS::PlayerInputSystem>();
    auto& waypointSys = registry.AddSystem<ECS::WaypointSystem>();
    auto& steeringSys = registry.AddSystem<ECS::SteeringSystem>();

    // Estos siempre están activos
    registry.AddSystem<ECS::CameraSystem>(g_window);
    registry.AddSystem<ECS::RenderSystem>(g_window);
    registry.AddSystem<ECS::UISystem>();

    // 2. Apagamos la lógica para iniciar en MODO EDICIÓN
    bool isPlaying = false;
    inputSys.SetEnabled(false);
    waypointSys.SetEnabled(false);
    steeringSys.SetEnabled(false);

    sf::Clock deltaClock;

    // --- JUGADOR (Kart) ---
    ECS::EntityID circle = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(circle, sf::Vector2f{ 400.f, 300.f });
    registry.AddComponent<ECS::Render>(circle, ECS::Render::Make(CIRCLE, sf::Color(100, 250, 50), "Textures/ColorChecker.png"));
    registry.AddComponent<ECS::SteeringAgent>(circle);
    registry.AddComponent<ECS::PlayerController>(circle);

    ECS::EntityID tri = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(tri, sf::Vector2f{ 200.f, 200.f }, 45.f);
    registry.AddComponent<ECS::Render>(tri, ECS::Render::Make(TRIANGLE, sf::Color::Cyan));

    // --- CÁMARA ---
    ECS::EntityID cam = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(cam, sf::Vector2f{ 0.f, 0.f });
    auto& camComp = registry.AddComponent<ECS::Camera>(cam);
    camComp.followTarget = circle;
    camComp.followSpeed = 5.f;
    camComp.zoom = 1;

    // --- IA: PURSUER ---
    ECS::EntityID enemigo = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(enemigo, sf::Vector2f{ 100.f, 100.f });
    registry.AddComponent<ECS::Render>(enemigo, ECS::Render::Make(TRIANGLE, sf::Color::Red));

    auto& iaAgent = registry.AddComponent<ECS::SteeringAgent>(enemigo);
    iaAgent.maxSpeed = 100.f;

    registry.AddComponent<ECS::SteeringBehavior>(enemigo);

    auto& path = registry.AddComponent<ECS::WaypointPath>(enemigo);
    path.points = {
        sf::Vector2f(100.f, 100.f),
        sf::Vector2f(700.f, 100.f),
        sf::Vector2f(700.f, 500.f),
        sf::Vector2f(100.f, 500.f)
    };
    path.isLoop = true;

    // --- IA: WANDERER ---
    ECS::EntityID wanderer = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(wanderer, sf::Vector2f{ 400.f, 300.f });
    registry.AddComponent<ECS::Render>(wanderer, ECS::Render::Make(CIRCLE, sf::Color::Green));
    registry.AddComponent<ECS::SteeringAgent>(wanderer);

    auto& wanderBehavior = registry.AddComponent<ECS::SteeringBehavior>(wanderer);
    wanderBehavior.wanderEnabled = true;

    /*auto& pursuerBehavior = registry.GetComponent<ECS::SteeringBehavior>(enemigo);
    pursuerBehavior.pursuitEnabled = true;
    pursuerBehavior.pursuitTarget = wanderer;
    */
    // --- OBSTÁCULO ---
    ECS::EntityID rock = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(rock, sf::Vector2f{ 300.f, 200.f });
    registry.AddComponent<ECS::Render>(rock, ECS::Render::Make(RECTANGLE, sf::Color::Yellow));
    registry.AddComponent<ECS::Obstacle>(rock);

    while (g_window.isOpen()) {
        while (const std::optional event =
            g_window.m_window->pollEvent()) {
            ImGui::SFML::ProcessEvent(*g_window.m_window, *event);

            if (event->is<sf::Event::Closed>()) {
                g_window.close();
            }
            else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                g_window.handleResize(resized->size);
            }
        }

        const sf::Time elapsedTime = deltaClock.restart();
        const float dt = elapsedTime.asSeconds();

        ImGui::SFML::Update(*g_window.m_window, elapsedTime);

        g_window.clear(sf::Color::Black);

        // Actualizamos el ECS
        registry.UpdateSystems(dt);

        // 3. VENTANA FLOTANTE DE PLAY / STOP
        ImGui::Begin("Controles de Juego", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::Button(isPlaying ? "Detener (Edicion)" : "Jugar (Play)", ImVec2(200, 40))) {
            isPlaying = !isPlaying;
            inputSys.SetEnabled(isPlaying);
            waypointSys.SetEnabled(isPlaying);
            steeringSys.SetEnabled(isPlaying);
        }
        ImGui::End();

        // 4. DIBUJAR VISUALMENTE LA RUTA EN MODO EDICIÓN
        if (!isPlaying) {
            auto view = registry.GetView<ECS::WaypointPath>();
            view.Each([&](ECS::EntityID, ECS::WaypointPath& path) {
                sf::CircleShape nodeShape(6.f);
                nodeShape.setOrigin(sf::Vector2f{ 6.f, 6.f });
                nodeShape.setFillColor(sf::Color::Magenta);

                for (const auto& point : path.points) {
                    nodeShape.setPosition(point);
                    g_window.draw(nodeShape); // Dibujamos directamente en la ventana
                }
                });
        }

        // Renderizar ImGui después de la escena.
        ImGui::SFML::Render(*g_window.m_window);

        // Presentar el frame.
        g_window.display();
    }

    destroy();

    return 0;
}