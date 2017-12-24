#pragma once

/**
 * @file Scene.hpp
 * @brief Scene header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vector>
#include <queue>
#include <memory>
#include "System.hpp"

namespace gust
{
	class Transform;

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
		 * @brief Create a new entity.
		 * @return Entity handle.
		 */
		size_t create();

		/**
		 * @brief Destroy an entity.
		 * @param Entity handle.
		 */
		void destroy(size_t handle);

		/**
		 * @brief Add a system to the scene.
		 * @tparam Type of system.
		 */
		template<class T>
		void addSystem()
		{
			static_assert(std::is_base_of<System, T>::value, "T must derive from gust::System");
			m_systems.push_back(std::make_unique<T>(this));
		}

		/**
		 * @brief Remove a component from the system acting upon the given type.
		 * @tparam Component type.
		 * @param Entity.
		 */
		template<class T>
		void removeComponent(Entity entity)
		{
			static_assert(std::is_base_of<Component<T>, T>::value, "T must derive from gust::Component");
			static_assert(!std::is_same<T, Transform>::value, "T must not be of type gust::Transform");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Get system of required type
			System* system = getSystemOfType<T>();

			if (system)
			{
				// Cast system as appropriate ResourceAllocator
				auto allocator = static_cast<ResourceAllocator<T>*>(system->m_components.get());

				// Loop over every component in the allocator
				for(size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
					if (allocator->isAllocated(i))
					{
						T* component = allocator->getResourceByHandle(i);
						assert(entity.getHandle() == component->getEntity().getHandle());
						size_t oldHandle = system->m_componentHandle;
						system->m_componentHandle = i;
						system->onEnd();
						allocator->deallocate(i);
						system->m_componentHandle = oldHandle;
					}
			}
		}

		/**
		 * @brief Add component to the system acting upon the given type.
		 * @tparam Component type.
		 * @param Entity the new component will belong to.
		 * @return New component.
		 * @note Will return nullptr if the system doesn't exist to hold the component.
		 * @note Will return a pointer to a prexisting component if it already exists.
		 */
		template<class T>
		Handle<T> addComponent(Entity entity)
		{
			static_assert(std::is_base_of<Component<T>, T>::value, "T must derive from gust::Component");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Get system of required type
			System* system = getSystemOfType<T>();

			if (system)
			{
				// Cast system as appropriate ResourceAllocator
				auto allocator = static_cast<ResourceAllocator<T>*>(system->m_components.get());

				// Resize the allocator if needed
				if (allocator->getResourceCount() == allocator->getMaxResourceCount())
					allocator->resize(allocator->getMaxResourceCount() + 100, true);

				// Loop over every component in the allocator to check if the component already exists
				for(size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
					if (allocator->isAllocated(i))
					{
						T* component = allocator->getResourceByHandle(i);
						if (component->getEntity().getHandle() == entity.getHandle())
							return Handle<T>(allocator, i);
					}

				// Allocate a new component
				size_t handle = allocator->allocate();

				// Call constructor
				Handle<T> componentHandle = Handle<T>(allocator, handle);
				T* component = allocator->getResourceByHandle(handle);
				::new(component)(T)(entity, componentHandle);

				// Call onBegin()		
				size_t oldHandle = system->m_componentHandle;
				system->m_componentHandle = handle;
				system->onBegin();
				system->m_componentHandle = oldHandle;

				return componentHandle;
			}

			return Handle<T>::nullHandle();
		}

		/**
		 * @brief Get component belonging to the given entity of the given type.
		 * @tparam Component type.
		 * @param Entity the component belongs to.
		 * @return Component belonging to the entity of the given type.
		 * @note Will return nullptr if the component doesn't exist or the system doesn't exist.
		 */
		template<class T>
		Handle<T> getComponent(Entity entity)
		{
			static_assert(std::is_base_of<Component<T>, T>::value, "T must derive from gust::Component");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Get system of required type
			System* system = getSystemOfType<T>();

			// Loop over every system checking what component it works with
			if(system)
			{
				// Cast system as appropriate ResourceAllocator
				auto allocator = static_cast<ResourceAllocator<T>*>(system->m_components.get());

				// Loop over every component and check for the one we want
				for(size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
					if (allocator->isAllocated(i))
					{
						T* component = allocator->getResourceByHandle(i);
						if (component->getEntity().getHandle() == entity.getHandle())
							return Handle<T>(allocator, i);
					}
			}

			return Handle<T>::nullHandle();
		}

	private:

		/**
		 * @brief Find the system acting upon the given type.
		 * @tparam Component type.
		 * @return System acting upon the given type.
		 * @note Will return nullptr if it doesn't exist.
		 */
		template<class T>
		System* getSystemOfType()
		{
			static_assert(std::is_base_of<Component<T>, T>::value, "T must derive from Component");

			// Get ID of the component
			size_t id = TypeID<T>::id();

			// Loop over every system checking what component it works with
			for (auto& system : m_systems)
				if (system->getID() == id)
					return system.get();

			return nullptr;
		}

		/**
		 * @brief Find the system acting upon the given type.
		 * @param Component ID.
		 * @return System acting upon the given type.
		 * @note Will return nullptr if it doesn't exist.
		 */
		System* getSystemOfType(size_t id);

		/**
		 * @brief Get every component belonging to a given entity.
		 * @param Entity to get components of.
		 * @return List of components belonging to the entity.
		 */
		std::vector<ComponentBase*> getComponentsOfEntity(Entity entity);

		/**
		 * @brief Destroy marked entities.
		 */
		void destroyMarkedEntities();

		/**
		 * @brief Destroy marked components.
		 */
		void destroyMarkedComponents();



		/** Vector of systems. */
		std::vector<std::unique_ptr<System>> m_systems = {};

		/** Entity handle counter. */
		size_t m_entityHandleCounter = 0;

		/** List of free entity handles. */
		std::queue<size_t> m_freeEntityHandles = {};

		/** List of components to destroy on the next tick. */
		std::vector<ComponentBase*> m_markedComponents = {};
		
		/** List of entities to destroy on the next tick. */
		std::vector<size_t> m_markedEntities = {};
	};
}