#pragma once

/**
 * @file Entity.hpp
 * @brief Entity header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "../Utilities/Allocators.hpp"

namespace gust
{
	class Scene;

	/**
	 * @class Entity
	 * @brief Entity handle.
	 */
	class Entity final
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Entity() = default;

		/**
		 * @brief Constructor.
		 * @param Scene the entity is in.
		 * @note This will create a new entity.
		 */
		Entity(Scene* scene);

		/**
		 * @brief Constructor.
		 * @param Scene the entity is in.
		 * @param Entities handle.
		 */
		Entity(Scene* scene, size_t handle);

		/**
		 * @brief Default destructor.
		 */
		~Entity() = default;

		/**
		 * @brief Equvilence check.
		 * @param Other.
		 * @return If the entities are the same.
		 */
		friend bool operator==(const Entity& lh, const Entity& rh)
		{
			return (lh.m_handle == rh.m_handle) && (lh.m_scene == rh.m_scene);
		}

		/**
		 * @brief Unequvilence check.
		 * @param Other.
		 * @return If the entities are not the same.
		 */
		friend bool operator!=(const Entity& lh, const Entity& rh)
		{
			return (lh.m_handle != rh.m_handle) || (lh.m_scene != rh.m_scene);
		}

		/**
		 * @brief Get scene the entity is in.
		 * @return Scene the entity is in.
		 */
		inline Scene* getScene() const
		{
			return m_scene;
		}

		/**
		 * @brief Get entities handle.
		 * @return Entities handle.
		 */
		inline size_t getHandle() const
		{
			return m_handle;
		}

		/**
		 * @brief Remove a component.
		 */
		template<class T>
		void removeComponent()
		{
			m_scene->removeComponent<T>(*this);
		}

		/**
		 * @brief Add component to the system acting upon the given type.
		 * @return New component.
		 * @note Will return nullptr if the system doesn't exist to hold the component.
		 * @note Will return a pointer to a prexisting component if it already exists.
		 */
		template<class T>
		Handle<T> addComponent()
		{
			return m_scene->addComponent<T>(*this);
		}

		/**
		 * @brief Get component belonging to the given entity of the given type.
		 * @return Component belonging to the entity of the given type.
		 * @note Will return nullptr if the component doesn't exist or the system doesn't exist.
		 */
		template<class T>
		Handle<T> getComponent()
		{
			return m_scene->getComponent<T>(*this);
		}

	private:

		/** Scene the entity is in. */
		Scene* m_scene;

		/** Entities handle. */
		size_t m_handle;
	};
}