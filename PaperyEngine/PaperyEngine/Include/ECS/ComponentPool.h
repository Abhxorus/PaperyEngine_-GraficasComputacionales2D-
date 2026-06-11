/**
 * @file ComponentPool.h
 * @brief Define las clases de gestión de pools de componentes para el Entity Component System (ECS).
 * * Contiene la interfaz polimórfica IComponentPool para gestionar pools heterogéneos
 * y la implementación genérica ComponentPool<T> que almacena los componentes en un
 * arreglo denso paralelo al SparseSet.
 */

#pragma once
#include "Prerequisites.h"
#include "SparseSet.h"

namespace ECS {

	/**
	 * @class IComponentPool
	 * @brief Interfaz polimórfica sin tipo para la gestión de componentes.
	 * * Permite que la clase Registry pueda gestionar múltiples pools de diferentes tipos
	 * de forma heterogénea a través de punteros a esta interfaz base.
	 */
	class IComponentPool : public SparseSet {
	public:
		/**
		 * @brief Destructor virtual por defecto.
		 */
		virtual ~IComponentPool() = default;

		/**
		 * @brief Elimina el componente asociado a una entidad si este existe.
		 * @param entity Identificador de la entidad cuyo componente se desea eliminar.
		 */
		virtual void RemoveEntity(EntityID entity) = 0;

		/**
		 * @brief Obtiene un puntero sin tipo (void*) al componente.
		 * * Útil para sistemas que requieren acceso genérico a los datos, como un Serializador.
		 * * @param entity Identificador de la entidad.
		 * @return Puntero al componente en crudo, o nullptr si la entidad no lo posee.
		 */
		virtual void* GetRaw(EntityID entity) noexcept = 0;
	};

	/**
	 * @class ComponentPool
	 * @brief Almacena componentes de tipo T en un arreglo denso paralelo al SparseSet.
	 * @tparam T El tipo de componente que almacenará este pool.
	 * * Utiliza un mecanismo de *swap-with-last* (intercambio con el último elemento)
	 * durante las eliminaciones para mantener el arreglo denso contiguo y sincronizado
	 * con los índices del SparseSet base.
	 */
	template<typename T>
	class ComponentPool final : public IComponentPool {
	public:
		/**
		 * @brief Añade un nuevo componente a la entidad.
		 * @tparam Args Tipos de los argumentos para construir el componente.
		 * @param entity La entidad a la que se le añadirá el componente.
		 * @param args Argumentos que se reenviarán al constructor del componente T.
		 * @return Referencia al componente recién creado.
		 * @pre La entidad no debe tener ya un componente de este tipo en el pool.
		 */
		template<typename... Args>
		T& Add(EntityID entity, Args&&... args) {
			assert(!Contains(entity) && "La entidad ya tiene este componente");
			InsertEntity(entity); // registra en sparse/dense
			m_components.emplace_back(std::forward<Args>(args)...);
			return m_components.back();
		}

		/**
		 * @brief Obtiene una referencia al componente de la entidad.
		 * @param entity La entidad propietaria del componente.
		 * @return Referencia modificable al componente.
		 * @pre La entidad debe poseer el componente en el pool.
		 */
		[[nodiscard]] T& Get(EntityID entity) noexcept {
			assert(Contains(entity) && "La entidad no tiene este componente");
			return m_components[m_sparse[GetEntityIndex(entity)]];
		}

		/**
		 * @brief Obtiene una referencia constante al componente de la entidad.
		 * @param entity La entidad propietaria del componente.
		 * @return Referencia de solo lectura al componente.
		 * @pre La entidad debe poseer el componente en el pool.
		 */
		[[nodiscard]] const T& Get(EntityID entity) const noexcept {
			assert(Contains(entity) && "La entidad no tiene este componente");
			return m_components[m_sparse[GetEntityIndex(entity)]];
		}

		/**
		 * @brief Intenta obtener un puntero al componente de la entidad.
		 * @param entity La entidad propietaria del componente.
		 * @return Puntero al componente si existe, o nullptr si la entidad no lo tiene.
		 */
		[[nodiscard]] T* TryGet(EntityID entity) noexcept {
			if (!Contains(entity)) return nullptr;
			return &m_components[m_sparse[GetEntityIndex(entity)]];
		}

		/**
		 * @brief Elimina el componente de la entidad utilizando el método swap-with-last.
		 * * Importante: Primero se sincroniza m_components y luego se llama a
		 * SparseSet::Remove para que sincronice m_dense. Ambos intercambios usan
		 * el mismo índice denso, manteniéndose alineados.
		 * * @param entity La entidad cuyo componente será eliminado.
		 */
		void Remove(EntityID entity) override {
			if (!Contains(entity)) return;

			const EntityIndex denseIdx = m_sparse[GetEntityIndex(entity)];

			// Mueve el último componente al hueco
			m_components[denseIdx] = std::move(m_components.back());
			m_components.pop_back();

			// Sincroniza sparse/dense (base class)
			SparseSet::Remove(entity);
		}

		/**
		 * @brief Elimina el componente asociado a una entidad si este existe.
		 * @param entity Identificador de la entidad.
		 */
		void RemoveEntity(EntityID entity) override {
			Remove(entity);
		}

		/**
		 * @brief Implementación de la interfaz para obtener el puntero en crudo.
		 * @param entity Identificador de la entidad.
		 * @return Puntero tipo void* al componente o nullptr.
		 */
		void* GetRaw(EntityID entity) noexcept override {
			return TryGet(entity);
		}

		/**
		 * @brief Proporciona acceso directo al arreglo interno de componentes.
		 * * Útil para accesos masivos y operaciones eficientes en sistemas o el Serializador.
		 * * @return Referencia al vector que almacena los componentes.
		 */
		[[nodiscard]] std::vector<T>& GetComponents() noexcept {
			return m_components;
		}

		/**
		 * @brief Proporciona acceso directo de solo lectura al arreglo interno de componentes.
		 * @return Referencia constante al vector que almacena los componentes.
		 */
		[[nodiscard]] const std::vector<T>& GetComponents() const noexcept {
			return m_components;
		}

		/**
		 * @brief Limpia todos los componentes y vacía el pool.
		 * * Llama a la limpieza de la clase base SparseSet y luego limpia el arreglo denso de componentes.
		 */
		void Clear() override {
			SparseSet::Clear();
			m_components.clear();
		}

	private:
		/**
		 * @brief Arreglo denso de componentes.
		 * * Sus elementos se mantienen paralelos y sincronizados con el arreglo `m_dense` de SparseSet.
		 */
		std::vector<T> m_components;
	};

} // namespace ECS