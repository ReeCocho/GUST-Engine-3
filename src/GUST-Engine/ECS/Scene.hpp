#pragma once

/**
 * @file Scene.hpp
 * @brief Scene header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vector>
#include <memory>
#include "System.hpp"

namespace gust
{
	/**
	 * @class Scene
	 * @brief Manages systems and entities.
	 */
	class Scene
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Scene() = default;

		/**
		 * @brief Default destructor.
		 */
		~Scene() = default;

		/**
		 * @brief Initialize scene.
		 * @note Used internally. Do not call.
		 */
		void startup();

		/**
		 * @brief Shut down scene.
		 * @note Used internally. Do not call.
		 */
		void shutdown();

		/**
		 * @brief Perform a tick in the scene.
		 * @param Delta time.
		 */
		void tick(float deltaTime);

		/**
		 * @brief Remove a component from the system acting upon the given type.
		 * @param Entity.
		 */
		template<class T>
		void removeComponent(Entity entity)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from gust::Component");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Get system of required type
			System<T>* system = getSystemOfType<T>();

			if (system)
			{
				// Cast system as appropriate ResourceAllocator
				ResourceAllocator<T>* allocator = &static_cast<System<T>>(system.get())->m_components;

				// Loop over every component in the allocator
				for(size_t i = 0; i < allocator->getMaxResourceCount(); i++)
					if (allocator->isAllocated(i))
					{
						T* component = allocator->getResourceByHandle(i);
						assert(allocator == component.getResourceAllocator() && entity.getHandle() == component->getEntity().getHandle());
						system->onEnd(Handle<T>(&allocator, i));
						allocator->deallocate(component.getHandle());
					}
			}
		}

		/**
		 * @brief Add component to the system acting upon the given type.
		 * @param Entity the new component will belong to.
		 * @return New component.
		 * @note Will return nullptr if the system doesn't exist to hold the component.
		 * @note Will return a pointer to a prexisting component if it already exists.
		 */
		template<class T>
		Handle<T> addComponent(Entity entity)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from gust::Component");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Get system of required type
			System<T>* system = getSystemOfType<T>();

			if (system)
			{
				// Cast system as appropriate ResourceAllocator
				ResourceAllocator<T>* allocator = &static_cast<System<T>>(system.get())->m_components;

				// Loop over every component in the allocator to check if the component already exists
				for(size_t i = 0; i < allocator->getMaxResourceCount(); i++)
					if (allocator->isAllocated(i))
					{
						T* component = allocator->getResourceByHandle(i);
						if (component->getEntity().getHandle() == entity.getHandle())
							return Handle<T>(&allocator, i);
					}

				// Allocate a new component
				size_t handle = allocator->allocate();

				// Call constructor
				T* component = allocator->getResourceByHandle(handle);
				::new(component)(T)(entity, TypeID<T>::id());

				// Call onBegin()
				Handle<T> componentHandle = Handle<T>(&allocator, handle);
				system->onBegin(componentHandle);

				return componentHandle;
			}

			return nullptr;
		}

		/**
		 * @brief Get component belonging to the given entity of the given type.
		 * @param Entity the component belongs to.
		 * @return Component belonging to the entity of the given type.
		 * @note Will return nullptr if the component doesn't exist or the system doesn't exist.
		 */
		template<class T>
		Handle<T> getComponent(Entity entity)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from gust::Component");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Get system of required type
			System<T>* system = getSystemOfType<T>();

			// Loop over every system checking what component it works with
			if(system)
			{
				// Cast system as appropriate ResourceAllocator
				ResourceAllocator<T>* allocator = &static_cast<System<T>>(system.get())->m_components;

				// Loop over every component and check for the one we want
				for(size_t i = 0; i < allocator->getMaxResourceCount(); i++)
					if (allocator->isAllocated(i))
					{
						T* component = allocator->getResourceByHandle(i);
						if (component->getEntity().getHandle() == entity.getHandle())
							return Handle<T>(allocator, i);
					}
			}

			return nullptr;
		}

	private:

		/**
		 * @brief Find the system acting upon the given type.
		 * @return System acting upon the given type.
		 * @note Will return nullptr if it doesn't exist.
		 */
		template<class T>
		System<T>* getSystemOfType()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Loop over every system checking what component it works with
			for (auto& system : m_systems)
				if (system->getID() == id)
					return static_cast<System<T>*>(system.get());

			return nullptr;
		}

		/** Vector of systems. */
		std::vector<std::unique_ptr<SystemBase>> m_systems = {};
	};
}