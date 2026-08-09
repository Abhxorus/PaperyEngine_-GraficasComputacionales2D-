#pragma once
#include "Prerequisites.h"
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/SteeringAgent.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/Obstacle.h"
#include "ECS/Components/WaypointPath.h"
#include "ECS/Components/Checkpoint.h"

// ============================================================
//  ECS :: Systems/UISystem.h
//
//  UISystem — LÓGICA de los paneles de ImGui.
//
//  IMPORTANTE: este sistema contiene SOLO los ImGui::Begin/End
//  y los widgets. El ciclo de frame de ImGui
//  (ImGui_ImplXXX_NewFrame + ImGui::NewFrame() al inicio, y
//   ImGui::Render() + RenderDrawData al final) NO va aquí:
//  vive en el game loop / en tu Window.
//
//  Por eso este sistema debe registrarse EL ÚLTIMO
//  (después del RenderSystem), para que la UI quede encima.
// ============================================================

namespace ECS {

    class UISystem final : public System {
    public:
        UISystem() = default;

        void OnStart(Registry& /*registry*/) override {
            // Habilitar docking.
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            setupDarkGUIStyle();
        }

        void
            OnUpdate(Registry& registry, float deltaTime) override {
            ImGuiDockNodeFlags dockspaceFlags =
                ImGuiDockNodeFlags_PassthruCentralNode;

            ImGui::DockSpaceOverViewport(
                0,
                ImGui::GetMainViewport(),
                dockspaceFlags
            );

            DrawOutliner(registry);
            DrawDetails(registry);
        }

        void
            DrawOutliner(Registry& registry) {
            ImGui::Begin("Outliner");
            {
                registry.GetView<ECS::Transform>().Each(
                    [this](ECS::EntityID id, ECS::Transform& /*transform*/) {
                        const std::string label = "Entity " + std::to_string(id);
                        const bool isSelected = (id == selectedEntity);

                        if (ImGui::Selectable(label.c_str(), isSelected))
                            selectedEntity = id;
                    });
            }
            ImGui::End();
        }

        void
            DrawDetails(Registry& registry) {
            ImGui::Begin("Details");
            {
                if (selectedEntity != ECS::NULL_ENTITY && registry.IsAlive(selectedEntity)) {
                    ImGui::Text("Entity %llu", static_cast<unsigned long long>(selectedEntity));
                    ImGui::Separator();

                    if (auto* t = registry.TryGetComponent<ECS::Transform>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                            vec2Control("Position", &t->position.x, 0, 120.0f);
                            vec2Control("Rotation", &t->rotation, 1.f, 120.0f);
                            vec2Control("Scale", &t->scale.x, 0.01f, 120.0f);
                            //ImGui::DragFloat2("Position", &t->position.x, 1.f);
                            //ImGui::DragFloat("Rotation", &t->rotation, 1.f);
                            //ImGui::DragFloat2("Scale", &t->scale.x, 0.01f);
                        }
                    }

                    if (auto* r = registry.TryGetComponent<ECS::Render>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Checkbox("Visible", &r->visible);

                            // sf::Color es 0..255; ImGui::ColorEdit4 trabaja en 0..1.
                            float color[4] = {
                              r->fillColor.r / 255.f,
                              r->fillColor.g / 255.f,
                              r->fillColor.b / 255.f,
                              r->fillColor.a / 255.f
                            };
                            if (ImGui::ColorEdit4("Fill Color", color)) {
                                r->fillColor = sf::Color(
                                    static_cast<std::uint8_t>(color[0] * 255.f),
                                    static_cast<std::uint8_t>(color[1] * 255.f),
                                    static_cast<std::uint8_t>(color[2] * 255.f),
                                    static_cast<std::uint8_t>(color[3] * 255.f));
                            }
                        }
                    }

                    if (auto* cam = registry.TryGetComponent<ECS::Camera>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                            // La posición y la rotación de la cámara se editan arriba,
                            // en el bloque Transform. Aquí solo lo propio de la cámara.
                            ImGui::Checkbox("Active", &cam->active);
                            ImGui::DragFloat("Zoom", &cam->zoom, 0.01f, 0.05f, 10.f);
                            ImGui::DragFloat("Follow Speed", &cam->followSpeed, 0.1f, 0.f, 50.f);

                            // Objetivo a seguir (solo lectura aquí; muestra el ID).
                            if (cam->followTarget == ECS::NULL_ENTITY)
                                ImGui::Text("Follow Target: (ninguno)");
                            else
                                ImGui::Text("Follow Target: %llu",
                                    static_cast<unsigned long long>(cam->followTarget));
                        }
                    }
                    if (auto* agent = registry.TryGetComponent<ECS::SteeringAgent>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Steering Agent", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::DragFloat("Max Speed", &agent->maxSpeed, 1.f, 10.f, 1000.f);
                            ImGui::DragFloat("Max Force", &agent->maxForce, 1.f, 1.f, 500.f);
                            ImGui::DragFloat("Mass", &agent->mass, 0.1f, 0.1f, 100.f);
                            vec2Control("Velocity", &agent->velocity.x, 0.f, 120.0f);
                        }
                    }

                    if (auto* behavior = registry.TryGetComponent<ECS::SteeringBehavior>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Steering Behaviors", ImGuiTreeNodeFlags_DefaultOpen)) {
                            vec2Control("Target", &behavior->target.x, 0.f, 120.0f);

                            ImGui::SeparatorText("Active Behaviors");
                            ImGui::Checkbox("Seek", &behavior->seekEnabled);
                            ImGui::Checkbox("Flee", &behavior->fleeEnabled);
                            ImGui::Checkbox("Arrive", &behavior->arriveEnabled);
                            ImGui::Checkbox("Wander", &behavior->wanderEnabled);
                            ImGui::Checkbox("Pursuit", &behavior->pursuitEnabled);
                            ImGui::Checkbox("Avoid Obstacles", &behavior->avoidanceEnabled);

                            ImGui::SeparatorText("Parameters");
                            if (behavior->arriveEnabled) ImGui::DragFloat("Slowing Radius", &behavior->slowingRadius, 1.f, 10.f, 500.f);
                            if (behavior->wanderEnabled) {
                                ImGui::DragFloat("Wander Radius", &behavior->wanderRadius, 1.f, 10.f, 200.f);
                                ImGui::DragFloat("Wander Dist", &behavior->wanderDistance, 1.f, 10.f, 300.f);
                                ImGui::DragFloat("Wander Jitter", &behavior->wanderJitter, 0.1f, 0.1f, 50.f);
                            }
                            if (behavior->avoidanceEnabled) ImGui::DragFloat("Feeler Length", &behavior->avoidanceFeelerLength, 1.f, 10.f, 500.f);
                            if (behavior->pursuitEnabled) {
                                ImGui::Text("Pursuit Target: %llu", static_cast<unsigned long long>(behavior->pursuitTarget));
                            }
                        }
                    }

                    if (auto* obs = registry.TryGetComponent<ECS::Obstacle>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Obstacle (Avoidance)", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::DragFloat("Radius", &obs->radius, 1.f, 1.f, 200.f);
                        }
                    }
                    if (auto* path = registry.TryGetComponent<ECS::WaypointPath>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Waypoint Path", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Checkbox("Loop Circuit", &path->isLoop);
                            ImGui::DragFloat("Reach Radius", &path->reachRadius, 1.f, 5.f, 200.f);

                            ImGui::SeparatorText("Ruta (Nodos)");
                            for (size_t i = 0; i < path->points.size(); ++i) {
                                ImGui::PushID(static_cast<int>(i));

                                // Usamos tu vec2Control para mantener el mismo estilo visual
                                vec2Control("Nodo " + std::to_string(i), &path->points[i].x, 0.f, 80.f);

                                // Botón para eliminar un nodo específico
                                if (ImGui::Button("Eliminar Nodo")) {
                                    path->points.erase(path->points.begin() + i);
                                    ImGui::PopID();
                                    break; // Rompemos el ciclo para evitar errores de iteración al modificar el vector
                                }
                                ImGui::Separator();
                                ImGui::PopID();
                            }

                            if (ImGui::Button("Añadir Nodo", ImVec2(-1, 30))) {
                                // Añade un nuevo nodo un poco desplazado del último para que sea visible
                                sf::Vector2f newPoint = path->points.empty() ?
                                    sf::Vector2f(0.f, 0.f) : path->points.back() + sf::Vector2f(50.f, 0.f);
                                path->points.push_back(newPoint);
                            }
                        }
                    }
                    if (auto* cp = registry.TryGetComponent<ECS::Checkpoint>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Checkpoint", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::DragInt("Orden (Index)", &cp->index, 1, 0, 100);
                            ImGui::DragFloat("Radio", &cp->radius, 1.f, 10.f, 1000.f);
                            ImGui::Checkbox("Es Linea de Meta", &cp->isFinishLine);
                        }
                    }
                }
                else {
                    ImGui::TextDisabled("Selecciona una entidad en el outliner.");
                }
            }
            ImGui::End();
        }

        void
            setupDarkGUIStyle() {
            ImGuiStyle& style = ImGui::GetStyle();

            style.Alpha = 1.0f;
            style.DisabledAlpha = 1.0f;
            style.WindowPadding = ImVec2(12.0f, 12.0f);
            style.WindowRounding = 0.0f;
            style.WindowBorderSize = 0.0f;
            style.WindowMinSize = ImVec2(20.0f, 20.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_None;
            style.ChildRounding = 0.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 0.0f;
            style.PopupBorderSize = 1.0f;
            style.FramePadding = ImVec2(6.0f, 6.0f);
            style.FrameRounding = 0.0f;
            style.FrameBorderSize = 0.0f;
            style.ItemSpacing = ImVec2(12.0f, 6.0f);
            style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
            style.CellPadding = ImVec2(12.0f, 6.0f);
            style.IndentSpacing = 20.0f;
            style.ColumnsMinSpacing = 6.0f;
            style.ScrollbarSize = 12.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabMinSize = 12.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 0.0f;
            style.TabBorderSize = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
        }

        void
            vec2Control(const std::string& label,
                float* values,
                float resetValues,
                float columnWidth) {
            ImGuiIO& io = ImGui::GetIO();
            auto boldFont = io.Fonts->Fonts[0];

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(label.c_str());
            ImGui::NextColumn();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
            ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

            float fullWidth = ImGui::CalcItemWidth();
            float itemWidth = fullWidth / 3.0f;

            // --- X Control ---
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            ImGui::PushFont(boldFont);
            if (ImGui::Button("X", buttonSize)) values[0] = resetValues;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::PushItemWidth(itemWidth);
            ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            // --- Y Control ---
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Y", buttonSize)) values[1] = resetValues;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::PushItemWidth(itemWidth);
            ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PopStyleVar();
            ImGui::Columns(1);
            ImGui::PopID();
        }

    private:
        // Entidad seleccionada en el outliner (NULL_ENTITY = ninguna).
        ECS::EntityID selectedEntity = ECS::NULL_ENTITY;
    };

};