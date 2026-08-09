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
#include "ECS/Components/PathFollower.h"
#include <fstream>
#include <iostream>

/**
 * @file main.cpp
 * @brief Punto de entrada principal de la aplicación y ciclo de ejecución del motor.
 */

 /** @brief Instancia global de la ventana de la aplicación. */
Window g_window(Window(800, 600, "Papery Engine"));
ECS::Registry registry;

/**
 * @brief Libera los recursos globales o realiza tareas de limpieza antes de cerrar la aplicación.
 */
void destroy()
{
    ImGui::SFML::Shutdown();
}

/**
 * @brief Función principal del programa.
 */
int main()
{
    if (!ImGui::SFML::Init(*g_window.m_window)) {
        return -1;
    }

    // 1. Guardamos las referencias de los sistemas lógicos al registrarlos
    auto& inputSys = registry.AddSystem<ECS::PlayerInputSystem>();
    auto& waypointSys = registry.AddSystem<ECS::WaypointSystem>();
    auto& steeringSys = registry.AddSystem<ECS::SteeringSystem>();
    auto& raceSys = registry.AddSystem<ECS::RaceSystem>(); // Sistema de conteo de vueltas

    // Estos siempre están activos
    registry.AddSystem<ECS::CameraSystem>(g_window);
    registry.AddSystem<ECS::RenderSystem>(g_window);
    registry.AddSystem<ECS::UISystem>();

    // 2. Apagamos la lógica para iniciar en MODO EDICIÓN
    bool isPlaying = false;
    inputSys.SetEnabled(false);
    waypointSys.SetEnabled(false);
    steeringSys.SetEnabled(false);
    raceSys.SetEnabled(false);

    sf::Clock deltaClock;

    // --- JUGADOR (Kart) ---
    ECS::EntityID circle = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(circle, sf::Vector2f{ 300.f, 300.f });
    registry.AddComponent<ECS::Render>(circle, ECS::Render::Make(RECTANGLE, sf::Color::White, "Bowser.png"));
    registry.AddComponent<ECS::SteeringAgent>(circle);
    registry.AddComponent<ECS::PlayerController>(circle);
    registry.AddComponent<ECS::RaceStats>(circle); // Necesario para competir

    // --- CÁMARA ---
    ECS::EntityID cam = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(cam, sf::Vector2f{ 0.f, 0.f });
    auto& camComp = registry.AddComponent<ECS::Camera>(cam);
    camComp.followTarget = circle;
    camComp.followSpeed = 5.f;
    camComp.zoom = 1;

    // --- 1. RUTA MAESTRA (Invisible) ---
    ECS::EntityID trackRuta = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(trackRuta, sf::Vector2f{ 0.f, 0.f });
    auto& path = registry.AddComponent<ECS::WaypointPath>(trackRuta);
    path.points = {
        sf::Vector2f(-20.f, 200.f),
        sf::Vector2f(-20.f, -20.f),
        sf::Vector2f(50.f, -130.f),
        sf::Vector2f(800.f, 200.f),
        sf::Vector2f(800.f, 650.f),
        sf::Vector2f(690.f, 690.f),
        sf::Vector2f(580.f, 650.f),
        sf::Vector2f(400.f, 360.f),
        sf::Vector2f(15.f, 520.f)
    };
    path.isLoop = true;
    path.reachRadius = 80.f;

    // --- 2. IA: ENEMIGO 1 ---
    ECS::EntityID enemigo1 = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(enemigo1, sf::Vector2f{ 100.f, 150.f });
    registry.AddComponent<ECS::Render>(enemigo1, ECS::Render::Make(TRIANGLE, sf::Color::Red));
    registry.AddComponent<ECS::RaceStats>(enemigo1); // Necesario para competir

    auto& ag1 = registry.AddComponent<ECS::SteeringAgent>(enemigo1);
    ag1.maxSpeed = 120.f;

    auto& beh1 = registry.AddComponent<ECS::SteeringBehavior>(enemigo1);
    beh1.avoidanceEnabled = true;

    auto& follower1 = registry.AddComponent<ECS::PathFollower>(enemigo1);
    follower1.pathEntity = trackRuta;
    follower1.laneOffsetMax = 40.f;

    registry.AddComponent<ECS::Obstacle>(enemigo1, ECS::Obstacle{ 30.f });

    // --- 3. IA: ENEMIGO 2 ---
    ECS::EntityID enemigo2 = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(enemigo2, sf::Vector2f{ 100.f, 250.f });
    registry.AddComponent<ECS::Render>(enemigo2, ECS::Render::Make(TRIANGLE, sf::Color::Magenta));
    registry.AddComponent<ECS::RaceStats>(enemigo2); // Necesario para competir

    auto& ag2 = registry.AddComponent<ECS::SteeringAgent>(enemigo2);
    ag2.maxSpeed = 110.f;

    auto& beh2 = registry.AddComponent<ECS::SteeringBehavior>(enemigo2);
    beh2.avoidanceEnabled = true;

    auto& follower2 = registry.AddComponent<ECS::PathFollower>(enemigo2);
    follower2.pathEntity = trackRuta;
    follower2.laneOffsetMax = 60.f;

    registry.AddComponent<ECS::Obstacle>(enemigo2, ECS::Obstacle{ 30.f });

    // --- OBSTÁCULO ESTÁTICO ---
    ECS::EntityID rock = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(rock, sf::Vector2f{ 300.f, 200.f });
    registry.AddComponent<ECS::Render>(rock, ECS::Render::Make(RECTANGLE, sf::Color::Yellow));
    registry.AddComponent<ECS::Obstacle>(rock);

    // --- MAPA (Fondo de la pista) ---
    ECS::EntityID mapEntity = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(mapEntity, sf::Vector2f{ 400.f, 300.f });

    auto mapShape = std::make_shared<sf::RectangleShape>();
    ECS::Render mapRender(mapShape);

    if (mapRender.SetTexture("Pista.jpg")) {
        sf::Vector2u texSize = mapRender.texture->getSize();
        sf::Vector2f fSize(static_cast<float>(texSize.x), static_cast<float>(texSize.y));
        mapShape->setSize(fSize);
        mapShape->setOrigin(fSize / 2.f);
    }
    else {
        mapShape->setSize(sf::Vector2f{ 800.f, 600.f });
        mapShape->setOrigin(sf::Vector2f{ 400.f, 300.f });
        mapShape->setFillColor(sf::Color::Magenta);
        std::cerr << "Error: No se pudo cargar el mapa.\n";
    }

    registry.AddComponent<ECS::Render>(mapEntity, mapRender);

    while (g_window.isOpen()) {
        while (const std::optional event = g_window.m_window->pollEvent()) {
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

        // --- VENTANA FLOTANTE ---
        ImGui::Begin("Controles de Juego", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button(isPlaying ? "Detener (Edicion)" : "Jugar (Play)", ImVec2(200, 40))) {
            isPlaying = !isPlaying;
            inputSys.SetEnabled(isPlaying);
            waypointSys.SetEnabled(isPlaying);
            steeringSys.SetEnabled(isPlaying);
            raceSys.SetEnabled(isPlaying);
        }

        ImGui::Separator();

        // Botón para generar código C++ y guardarlo en un TXT
        if (ImGui::Button("Exportar Pista a TXT", ImVec2(200, 30))) {
            std::ofstream file("pista_guardada.txt");
            file << "// ==========================================\n";
            file << "// Copia y pega esto en tu main.cpp\n";
            file << "// ==========================================\n\n";

            file << "// --- CHECKPOINTS ---\n";
            registry.GetView<ECS::Transform, ECS::Checkpoint>().Each([&](ECS::EntityID id, ECS::Transform& t, ECS::Checkpoint& cp) {
                file << "{\n";
                file << "    ECS::EntityID cp = registry.CreateEntity();\n";
                file << "    registry.AddComponent<ECS::Transform>(cp, sf::Vector2f{" << t.position.x << "f, " << t.position.y << "f});\n";
                if (cp.isFinishLine) {
                    file << "    registry.AddComponent<ECS::Render>(cp, ECS::Render::Make(CIRCLE, sf::Color(0, 255, 0, 100)));\n";
                }
                else {
                    file << "    registry.AddComponent<ECS::Render>(cp, ECS::Render::Make(CIRCLE, sf::Color(255, 255, 255, 100)));\n";
                }
                file << "    auto& chk = registry.AddComponent<ECS::Checkpoint>(cp);\n";
                file << "    chk.index = " << cp.index << ";\n";
                file << "    chk.radius = " << cp.radius << "f;\n";
                file << "    chk.isFinishLine = " << (cp.isFinishLine ? "true" : "false") << ";\n";
                file << "}\n\n";
                });

            file << "// --- NODOS DE IA ---\n";
            registry.GetView<ECS::WaypointPath>().Each([&](ECS::EntityID, ECS::WaypointPath& path) {
                file << "path.points = {\n";
                for (size_t i = 0; i < path.points.size(); ++i) {
                    file << "    sf::Vector2f(" << path.points[i].x << "f, " << path.points[i].y << "f)";
                    if (i < path.points.size() - 1) file << ",";
                    file << "\n";
                }
                file << "};\n";
                });

            file.close();
            std::cout << "Pista guardada con exito en pista_guardada.txt\n";
        }

        ImGui::End();

        // --- DIBUJAR VISUALMENTE LA RUTA EN MODO EDICIÓN ---
        if (!isPlaying) {
            auto view = registry.GetView<ECS::WaypointPath>();
            view.Each([&](ECS::EntityID, ECS::WaypointPath& path) {
                sf::CircleShape nodeShape(6.f);
                nodeShape.setOrigin(sf::Vector2f{ 6.f, 6.f });
                nodeShape.setFillColor(sf::Color::Magenta);

                for (const auto& point : path.points) {
                    nodeShape.setPosition(point);
                    g_window.draw(nodeShape);
                }
                });
        }

        ImGui::SFML::Render(*g_window.m_window);
        g_window.display();
    }

    destroy();
    return 0;
}