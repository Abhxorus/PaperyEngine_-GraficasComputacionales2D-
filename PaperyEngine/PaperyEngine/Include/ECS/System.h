#pragma once

/**
 * @file System.h
 * @brief Definición de la clase base System para el Entity Component System (ECS).
 */

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

    /**
     * @class System
     * @brief Interfaz base para todos los sistemas del motor.
     * @details Los sistemas encapsulan la lógica que opera sobre los datos almacenados
     * en los componentes de las entidades. Esta clase define su ciclo de vida.
     */
    class
        System {
    public:
        /**
         * @brief Destructor virtual por defecto.
         */
        virtual ~System() = default;

        // Inicialización: reservar recursos, suscribirse a eventos, etc.
        /**
         * @brief Método de inicialización llamado al registrar el sistema.
         * @param registry Referencia al Registry principal del ECS.
         */
        virtual void OnStart(Registry& /*registry*/) {}

        // Lógica frame-a-frame
        /**
         * @brief Método principal de actualización ejecutado en cada frame.
         * @param registry Referencia al Registry para interactuar con entidades y componentes.
         * @param deltaTime Tiempo transcurrido desde el último frame.
         */
        virtual void OnUpdate(Registry& registry, float deltaTime) = 0;

        // Limpieza al destruir el sistema
        /**
         * @brief Método de limpieza llamado al destruir o desregistrar el sistema.
         * @param registry Referencia al Registry principal del ECS.
         */
        virtual void OnDestroy(Registry& /*registry*/) {}

        // Opcional: activa/desactiva el sistema sin destruirlo
        /**
         * @brief Habilita o deshabilita la ejecución del sistema.
         * @param enabled true para activar el sistema, false para desactivarlo (ignorar en Update).
         */
        void SetEnabled(bool enabled) noexcept { m_enabled = enabled; }

        /**
         * @brief Comprueba si el sistema se encuentra habilitado para ejecutarse.
         * @return true si el sistema está activo, false en caso contrario.
         */
        [[nodiscard]] bool IsEnabled() const noexcept { return m_enabled; }

    private:
        /** @brief Bandera interna que determina si el sistema ejecutará su OnUpdate. */
        bool m_enabled = true;
    };

} // namespace ECS