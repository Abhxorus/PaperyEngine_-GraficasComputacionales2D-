#pragma once

// ============================================================
//  ECS :: System.h
//  Clase base para todos los sistemas del motor.
//
//  Un Sistema contiene únicamente LÓGICA, nunca datos.
//  Los datos viven en los componentes.
//
//  Ciclo de vida:
//    OnStart  → llamado una vez al registrar el sistema
//    OnUpdate → llamado cada frame
//    OnStop   → llamado al destruir o desregistrar el sistema
// ============================================================

namespace ECS {

    // Forward declaration para evitar inclusión circular
    class Registry;

    class
        System {
    public:
        virtual ~System() = default;

        // Inicialización: reservar recursos, suscribirse a eventos, etc.
        virtual void OnStart(Registry& /*registry*/) {}

        // Lógica frame-a-frame
        virtual void OnUpdate(Registry& registry, float deltaTime) = 0;

        // Limpieza al destruir el sistema
        virtual void OnDestroy(Registry& /*registry*/) {}

        // Opcional: activa/desactiva el sistema sin destruirlo
        void SetEnabled(bool enabled) noexcept { m_enabled = enabled; }
        [[nodiscard]] bool IsEnabled() const noexcept { return m_enabled; }

    private:
        bool m_enabled = true;
    };

} // namespace ECS