#pragma once
#include "ECS/ComponentPool.h"

/**
 * @file View.h
 * @brief Implementación de la clase View para realizar consultas (queries) sobre entidades y componentes en el ECS.
 */

namespace ECS {
    /**
     * @class View
     * @brief Permite iterar eficientemente sobre entidades que poseen un conjunto específico de componentes.
     * @tparam Components Lista de tipos de componentes que una entidad debe tener para ser incluida en la vista.
     */
    template<typename... Components>
    class View {
    public:
        /**
         * @brief Constructor de la vista.
         * Inicializa la tupla de pools y busca automáticamente el pool con menos elementos para optimizar la iteración.
         * @param pools Punteros a los ComponentPool de los tipos de componentes solicitados.
         */
        explicit View(ComponentPool<Components>*... pools) noexcept
            : m_pools(pools...) {
            FindSmallest();
        }

        // ── Iteración principal ───────────────────────────────
        // Callback: void(EntityID, Components&...)
        /**
         * @brief Ejecuta un callback por cada entidad que contenga todos los componentes requeridos.
         * @details Utiliza el pool más pequeño como base para la iteración y realiza la validación
         * hacia atrás, lo cual lo hace seguro en caso de eliminar componentes durante el recorrido.
         * @tparam Func Tipo de la función o lambda (deducido automáticamente).
         * @param func Función que se invocará con el EntityID y las referencias a sus componentes.
         */
        template<typename Func>
        void Each(Func&& func)
        {
            if (!m_smallest) return;

            const auto& entities = m_smallest->GetEntities();

            // Recorrido inverso → seguro al eliminar durante la iteración
            for (std::size_t i = entities.size(); i > 0; --i)
            {
                const EntityID entity = entities[i - 1];
                if (AllHave(entity))
                {
                    std::apply(
                        [&](auto*... pools) {
                            func(entity, pools->Get(entity)...);
                        },
                        m_pools);
                }
            }
        }

        // ── Iteración solo de entidades ───────────────────────
            // Útil cuando solo necesitas el EntityID y accedes a
            // componentes manualmente.
        /**
         * @brief Ejecuta un callback proporcionando únicamente el ID de la entidad válida.
         * @tparam Func Tipo de la función o lambda.
         * @param func Función que recibirá el identificador de la entidad (EntityID).
         */
        template<typename Func>
        void EachEntity(Func&& func)
        {
            if (!m_smallest) return;
            const auto& entities = m_smallest->GetEntities();
            for (std::size_t i = entities.size(); i > 0; --i)
            {
                const EntityID entity = entities[i - 1];
                if (AllHave(entity))
                    func(entity);
            }
        }

        /**
         * @brief Comprueba si la vista está vacía (no hay entidades que cumplan la firma).
         * @return true si la vista está vacía o algún pool requerido no existe, false en caso contrario.
         */
        [[nodiscard]] bool  Empty() const noexcept { return !m_smallest || m_smallest->Empty(); }

        /**
         * @brief Obtiene el tamaño máximo posible de entidades válidas en esta vista.
         * @return El número de elementos del pool más pequeño de la consulta.
         */
        [[nodiscard]] std::size_t Size() const noexcept { return m_smallest ? m_smallest->Size() : 0; }


    private:
        // Encuentra el pool con menos elementos (mejor filtro)
        /**
         * @brief Busca de forma recursiva (en tiempo de compilación/ejecución) el pool con menos entidades.
         * Esto optimiza drásticamente la iteración al reducir el número inicial de entidades a comprobar.
         * @tparam I Índice actual de la recursión sobre la tupla de pools.
         */
        template<std::size_t I = 0>
        void FindSmallest() noexcept {
            if constexpr (I < sizeof...(Components)) {
                auto* pool = std::get<I>(m_pools);
                if (pool && (!m_smallest || pool->Size() < m_smallest->Size()))
                    m_smallest = pool;
                FindSmallest<I + 1>();
            }
        }

        /**
         * @brief Verifica que una entidad específica se encuentre presente en todos los pools de la vista.
         * @param entity El identificador de la entidad a verificar.
         * @return true si la entidad tiene todos los componentes de la firma, false en caso contrario.
         */
        [[nodiscard]] bool AllHave(EntityID entity) const noexcept
        {
            return std::apply(
                [entity](auto*... pools) noexcept {
                    return (... && (pools && pools->Contains(entity)));
                },
                m_pools);
        }

    private:
        /** @brief Tupla que almacena los punteros a los pools de componentes involucrados en la vista. */
        std::tuple<ComponentPool<Components>*...> m_pools;

        /** @brief Puntero al pool con el menor número de elementos, usado como base para iterar de manera óptima. */
        const SparseSet* m_smallest = nullptr;
    };
}