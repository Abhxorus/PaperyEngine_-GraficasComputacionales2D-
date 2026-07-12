#pragma once
#include "Prerequisites.h"
#include "ECS/Types.h"

/**
 * @file SparseSet.h
 * @brief Estructura de datos Sparse Set para la gestión de entidades en el ECS.
 */

namespace ECS {

	/**
	 * @class SparseSet
	 * @brief Implementa un conjunto disperso (Sparse Set) que permite búsquedas O(1) e iteración contigua en memoria.
	 */
	class
		SparseSet {
	public:
		/**
		 * @brief Constructor por defecto de la clase SparseSet.
		 */
		SparseSet() = default;

		/**
		 * @brief Destructor virtual por defecto.
		 */
		virtual ~SparseSet() = default;

		// Consultas
		/**
		 * @brief Comprueba si una entidad se encuentra registrada en el Sparse Set.
		 * @param entity El identificador de la entidad a consultar.
		 * @return true si la entidad está presente, false en caso contrario.
		 */
		[[nodiscard]] bool Contains(EntityID entity) const noexcept
		{
			const EntityIndex idx = GetEntityIndex(entity);
			if (idx >= m_sparse.size()) return false;
			const EntityIndex denseIdx = m_sparse[idx];
			return denseIdx < m_dense.size() && m_dense[denseIdx] == entity;
		}

		/**
		 * @brief Obtiene la cantidad de entidades actualmente almacenadas.
		 * @return El número de elementos en el arreglo denso.
		 */
		[[nodiscard]] size_t Size()  const noexcept { return m_dense.size(); }

		/**
		 * @brief Verifica si el conjunto está vacío.
		 * @return true si no hay entidades almacenadas, false de lo contrario.
		 */
		[[nodiscard]] bool   Empty() const noexcept { return m_dense.empty(); }

		/**
		 * @brief Devuelve el arreglo denso con todas las entidades almacenadas.
		 * @return Referencia constante al vector de IDs de entidades.
		 */
		[[nodiscard]] const std::vector<EntityID>& GetEntities() const noexcept
		{
			return m_dense;
		}

		// ── Eliminación (swap-with-last) ──────────────────────
		// Las subclases DEBEN llamar a esta base DESPUÉS de
		// sincronizar sus propios arrays (ver ComponentPool::Remove).
		/**
		 * @brief Elimina una entidad del conjunto utilizando la técnica swap-with-last.
		 * @note Las subclases DEBEN llamar a este método base DESPUÉS de
		 * sincronizar sus propios arreglos.
		 * @param entity La entidad a eliminar.
		 */
		virtual void Remove(EntityID entity)
		{
			if (!Contains(entity)) return;

			const EntityIndex sparseIdx = GetEntityIndex(entity);
			const EntityIndex denseIdx = m_sparse[sparseIdx];
			const EntityID    last = m_dense.back();

			// Mueve el último elemento al hueco
			m_dense[denseIdx] = last;
			m_sparse[GetEntityIndex(last)] = denseIdx;
			m_dense.pop_back();

			// Invalida la entrada eliminada
			m_sparse[sparseIdx] = INVALID;
		}

		/**
		 * @brief Limpia por completo el Sparse Set, vaciando ambos arreglos.
		 */
		virtual void Clear()
		{
			m_sparse.clear();
			m_dense.clear();
		}

	protected:
		// Reserva espacio en m_sparse y registra la entidad en m_dense.
		// Devuelve el denseIndex asignado.
		/**
		 * @brief Inserta una nueva entidad en la estructura.
		 * @details Reserva espacio en el arreglo disperso (m_sparse) si es necesario y
		 * registra la entidad al final del arreglo denso (m_dense).
		 * @param entity La entidad a insertar.
		 * @return El índice denso (denseIndex) asignado a la entidad.
		 */
		EntityIndex InsertEntity(EntityID entity)
		{
			const EntityIndex sparseIdx = GetEntityIndex(entity);
			const EntityIndex denseIdx = static_cast<EntityIndex>(m_dense.size());

			if (sparseIdx >= m_sparse.size())
				m_sparse.resize(sparseIdx + 1, INVALID);

			assert(m_sparse[sparseIdx] == INVALID && "La entidad ya está en el set");

			m_sparse[sparseIdx] = denseIdx;
			m_dense.push_back(entity);
			return denseIdx;
		}

	protected:
		/** @brief Constante interna que representa un índice inválido en el arreglo disperso. */
		static constexpr EntityIndex INVALID = std::numeric_limits<EntityIndex>::max();

		/** @brief Arreglo disperso: mapea índices de entidad (EntityIndex) a índices densos. */
		std::vector<EntityIndex> m_sparse;   // sparse[entityIndex] → dense index

		/** @brief Arreglo denso: almacena los EntityID de manera contigua en memoria. */
		std::vector<EntityID>    m_dense;    // dense[i] → EntityID
	};
}