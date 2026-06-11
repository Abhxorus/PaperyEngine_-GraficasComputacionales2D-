#pragma once
#include "ECS/Types.h"
#include "ECS/ComponentPool.h"
#include "ECS/View.h"
#include "ECS/System.h"

/**
 * @file Registry.h
 * @brief Definición de la clase Registry para el Entity Component System (ECS).
 */

namespace ECS {
    /**
     * @brief Clase central del ECS que administra entidades, componentes y sistemas.
     */
    class
        Registry {
    public:
        /**
         * @brief Crea una nueva entidad reciclando índices libres o creando uno nuevo.
         * @return Identificador único de la entidad creada (EntityID).
         */
        EntityID CreateEntity() {
            EntityIndex idx;
            if (!m_freeList.empty()) {
                idx = m_freeList.front();
                m_freeList.pop();
            }
            else {
                idx = static_cast<EntityIndex>(m_versions.size());
                m_versions.push_back(0);
                m_entities.push_back(NULL_ENTITY);   // placeholder
            }

            EntityID id = MakeEntityID(idx, m_versions[idx]);
            m_entities[idx] = id;
            return id;
        }

        /**
         * @brief Destruye una entidad e invalida su ID.
         * @param entity El identificador de la entidad a destruir.
         */
        void
            DestroyEntity(EntityID entity) {
            assert(IsAlive(entity) && "DestroyEntity: entidad inválida o ya destruida");

            // Elimina todos los componentes de esta entidad
            for (auto& [typeID, pool] : m_componentPools)
                pool->RemoveEntity(entity);

            // Incrementa versión → los IDs viejos quedan inválidos
            const EntityIndex idx = GetEntityIndex(entity);
            ++m_versions[idx];
            m_entities[idx] = NULL_ENTITY;
            m_freeList.push(idx);
        }

        /**
         * @brief Verifica si una entidad sigue viva y es válida.
         * @param entity El identificador de la entidad.
         * @return true si la entidad es válida, false en caso contrario.
         */
        [[nodiscard]] bool
            IsAlive(EntityID entity) const noexcept {
            const EntityIndex idx = GetEntityIndex(entity);
            return idx < m_entities.size() && m_entities[idx] == entity;
        }

        /**
         * @brief Obtiene la cantidad de entidades activas.
         * @return Número de entidades vivas.
         */
        [[nodiscard]] std::size_t
            EntityCount() const noexcept {
            return m_entities.size() - m_freeList.size();
        }

        /**
         * @brief Todas las ranuras (incluye NULL_ENTITY para los huecos libres).
         * Útil para el Serializer; filtra con IsAlive.
         * @return Referencia al vector de todas las entidades.
         */
        [[nodiscard]] const std::vector<EntityID>&
            GetAllEntities() const noexcept {
            return m_entities;
        }

        //  Componentes

        /**
         * @brief Añade un componente a la entidad y devuelve su referencia.
         * Acepta argumentos de construcción directos (perfect-forward).
         * @tparam T Tipo de componente a añadir.
         * @tparam Args Tipos de los argumentos del constructor del componente.
         * @param entity La entidad objetivo.
         * @param args Argumentos para construir el componente.
         * @return Referencia al componente recién creado.
         */
        template<typename T, typename... Args> T&
            AddComponent(EntityID entity, Args&&... args) {
            assert(IsAlive(entity) && "AddComponent: entidad inválida");
            return GetOrCreatePool<T>()->Add(entity, std::forward<Args>(args)...);
        }

        /**
         * @brief Elimina el componente T de la entidad (no-op si no lo tiene).
         * @tparam T Tipo de componente a eliminar.
         * @param entity La entidad objetivo.
         */
        template<typename T> void
            RemoveComponent(EntityID entity) {
            if (auto* pool = GetPool<T>())
                pool->Remove(entity);
        }

        /**
         * @brief Reemplaza el componente (o lo añade si no existía).
         * @tparam T Tipo del componente.
         * @param entity La entidad objetivo.
         * @param value El valor del componente a asignar.
         * @return Referencia al componente modificado o añadido.
         */
        template<typename T>
        T& SetComponent(EntityID entity, T value) {
            assert(IsAlive(entity) && "SetComponent: entidad inválida");
            auto* pool = GetOrCreatePool<T>();
            if (pool->Contains(entity)) {
                pool->Get(entity) = std::move(value);
                return pool->Get(entity);
            }
            return pool->Add(entity, std::move(value));
        }

        /**
         * @brief Verifica si la entidad tiene un componente específico.
         * @tparam T Tipo del componente a verificar.
         * @param entity La entidad objetivo.
         * @return true si la entidad tiene el componente, false si no.
         */
        template<typename T>
        [[nodiscard]] bool HasComponent(EntityID entity) const noexcept {
            const auto* pool = GetPoolConst<T>();
            return pool && pool->Contains(entity);
        }

        /**
         * @brief Acceso garantizado al componente (assert si no existe).
         * @tparam T Tipo del componente a obtener.
         * @param entity La entidad objetivo.
         * @return Referencia al componente.
         */
        template<typename T>
        [[nodiscard]] T& GetComponent(EntityID entity) {
            assert(IsAlive(entity));
            auto* pool = GetPool<T>();
            assert(pool && "GetComponent: pool no existe para este tipo");
            return pool->Get(entity);
        }

        /**
         * @brief Acceso garantizado de solo lectura al componente (assert si no existe).
         * @tparam T Tipo del componente a obtener.
         * @param entity La entidad objetivo.
         * @return Referencia constante al componente.
         */
        template<typename T>
        [[nodiscard]] const T& GetComponent(EntityID entity) const
        {
            assert(IsAlive(entity));
            const auto* pool = GetPoolConst<T>();
            assert(pool && "GetComponent: pool no existe para este tipo");
            return pool->Get(entity);
        }

        /**
         * @brief Acceso seguro: devuelve nullptr si la entidad no tiene el componente.
         * @tparam T Tipo del componente a buscar.
         * @param entity La entidad objetivo.
         * @return Puntero al componente o nullptr si no lo posee.
         */
        template<typename T>
        [[nodiscard]] T* TryGetComponent(EntityID entity) noexcept
        {
            auto* pool = GetPool<T>();
            return pool ? pool->TryGet(entity) : nullptr;
        }

        //  Views (queries multi-componente)
        // Ejemplo: registry.GetView<Transform, Velocity>()
        /**
         * @brief Obtiene una vista para iterar sobre entidades que tienen ciertos componentes.
         * Ejemplo: registry.GetView<Transform, Velocity>()
         * @tparam Components Tipos de los componentes requeridos.
         * @return Un objeto View configurado.
         */
        template<typename... Components>
        [[nodiscard]] View<Components...> GetView() {
            return View<Components...>(GetOrCreatePool<Components>()...);
        }

        //  Sistemas
        /**
         * @brief Añade un sistema al Registry y ejecuta su inicialización.
         * @tparam T Tipo del sistema (debe derivar de ECS::System).
         * @tparam Args Tipos de los argumentos del constructor del sistema.
         * @param args Argumentos para construir el sistema.
         * @return Referencia al sistema instanciado.
         */
        template<typename T, typename... Args>
        T& AddSystem(Args&&... args)
        {
            static_assert(std::is_base_of_v<System, T>, "T debe derivar de ECS::System");
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *system;
            system->OnStart(*this);
            m_systems.push_back(std::move(system));
            return ref;
        }

        /**
         * @brief Actualiza todos los sistemas activos.
         * @param deltaTime Tiempo transcurrido en el frame.
         */
        void UpdateSystems(float deltaTime)
        {
            for (auto& system : m_systems)
                if (system->IsEnabled())
                    system->OnUpdate(*this, deltaTime);
        }

        /**
         * @brief Remueve y destruye todos los sistemas registrados.
         */
        void RemoveAllSystems()
        {
            for (auto& system : m_systems)
                system->OnDestroy(*this);
            m_systems.clear();
        }

        //  Utilidades
        // Destruye todo: entidades, componentes y sistemas.
        /**
         * @brief Limpia el Registry por completo.
         * Destruye todo: entidades, componentes y sistemas.
         */
        void
            Clear() {
            RemoveAllSystems();
            for (auto& [typeID, pool] : m_componentPools)
                pool->Clear();
            m_entities.clear();
            m_versions.clear();
            while (!m_freeList.empty()) m_freeList.pop();
        }

        // Acceso a pools sin tipo (para el Serializer)
        /**
         * @brief Acceso a pools sin tipo (principalmente para el Serializer).
         * @return Mapa de pools activos por ComponentTypeID.
         */
        [[nodiscard]] const std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>>&
            GetPools() const noexcept { return m_componentPools; }

    private:
        // ── Helpers privados ──────────────────────────────────

        /**
         * @brief Obtiene el pool para el componente T, creándolo si no existe.
         * @tparam T Tipo del componente.
         * @return Puntero al ComponentPool de T.
         */
        template<typename T>
        ComponentPool<T>* GetOrCreatePool() {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_componentPools.find(typeID);
            if (it == m_componentPools.end())
            {
                auto [newIt, ok] = m_componentPools.emplace(
                    typeID, std::make_unique<ComponentPool<T>>());
                return static_cast<ComponentPool<T>*>(newIt->second.get());
            }
            return static_cast<ComponentPool<T>*>(it->second.get());
        }

        /**
         * @brief Obtiene el pool para el componente T si existe.
         * @tparam T Tipo del componente.
         * @return Puntero al ComponentPool de T, o nullptr si no existe.
         */
        template<typename T>
        ComponentPool<T>* GetPool() noexcept {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_componentPools.find(typeID);
            return (it != m_componentPools.end())
                ? static_cast<ComponentPool<T>*>(it->second.get())
                : nullptr;
        }

        /**
         * @brief Obtiene el pool de solo lectura para el componente T si existe.
         * @tparam T Tipo del componente.
         * @return Puntero constante al ComponentPool de T, o nullptr si no existe.
         */
        template<typename T>
        const ComponentPool<T>* GetPoolConst() const noexcept {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_componentPools.find(typeID);
            return (it != m_componentPools.end())
                ? static_cast<const ComponentPool<T>*>(it->second.get())
                : nullptr;
        }

    private:
        // ── Entidades ─────────────────────────────────────────
        /** @brief Arreglo principal que almacena los IDs de las entidades. */
        std::vector<EntityID>      m_entities;
        /** @brief Versiones actuales de cada ranura de entidad. */
        std::vector<EntityVersion> m_versions;
        /** @brief Cola de índices reciclados listos para reutilizarse. */
        std::queue<EntityIndex>    m_freeList;

        // ── Componentes ───────────────────────────────────────
        /** @brief Mapa de pools de componentes identificados por ComponentTypeID. */
        std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>> m_componentPools;

        // ── Sistemas ──────────────────────────────────────────
        /** @brief Lista de sistemas activos en el ECS. */
        std::vector<std::unique_ptr<System>> m_systems;
    };
}