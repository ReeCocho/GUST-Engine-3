#pragma once

/**
 * @file System.hpp
 * @brief System header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <functional>
#include <memory>
#include "../Utilities/Allocators.hpp"
#include "Component.hpp"

namespace gust
{
	/**
	 * @class System
	 * @brief Manages components and running game code.
	 */
	class System
	{
		friend class Scene;

	public:

		/**
		 * @brief Default constructor.
		 */
		System() = default;

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		System(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		virtual ~System();

		/**
		 * @brief Initialize the system for use.
		 * @tparam Component type the system will work with.
		 */
		template<class T>
		void initialize()
		{
			if (m_id == 0)
			{
				m_id = TypeID<T>::id();
				m_components = std::make_unique<ResourceAllocator<T>>(50, alignof(T));

				m_runOnTick = [this](float deltaTime) 
				{ 
					auto allocator = static_cast<ResourceAllocator<T>*>(m_components.get());

					for(size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
						if (allocator->isAllocated(i))
						{
							m_componentHandle = i;
							onTick(deltaTime);
						}
				};

				m_runOnLateTick = [this](float deltaTime)
				{
					auto allocator = static_cast<ResourceAllocator<T>*>(m_components.get());

					for (size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
						if (allocator->isAllocated(i))
						{
							m_componentHandle = i;
							onLateTick(deltaTime);
						}
				};

				m_runOnPreRender = [this](float deltaTime)
				{
					auto allocator = static_cast<ResourceAllocator<T>*>(m_components.get());

					for (size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
						if (allocator->isAllocated(i))
						{
							m_componentHandle = i;
							onPreRender(deltaTime);
						}
				};

				m_destroyByEntity = [this](Entity entity)
				{
					auto allocator = static_cast<ResourceAllocator<T>*>(m_components.get());

					for (size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
						if (allocator->isAllocated(i))
						{
							T* component = allocator->getResourceByHandle(i);
							if (component->getEntity() == entity)
							{
								m_componentHandle = i;
								onEnd();
								allocator->deallocate(i);
							}
						}
				};

				m_destroyAllComponents = [this]()
				{
					auto allocator = static_cast<ResourceAllocator<T>*>(m_components.get());

					for (size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
						if (allocator->isAllocated(i))
						{
							m_componentHandle = i;
							onEnd();
							allocator->deallocate(i);
						}
				};

				m_componentByEntity = [this](Entity entity)
				{
					auto allocator = static_cast<ResourceAllocator<T>*>(m_components.get());

					ComponentBase* component = nullptr;

					for (size_t i = 0; i < allocator->getMaxResourceCount(); ++i)
						if (allocator->isAllocated(i))
						{
							component = static_cast<ComponentBase*>(allocator->getResourceByHandle(i));
							if (component->getEntity() == entity)
								return component;
						}

					return component;
				};
			}
		}

		/**
		 * @brief Get ID of the component being acted upon.
		 * @return ID of the component being acted upon.
		 */
		inline size_t getID() const
		{
			return m_id;
		}

		/**
		 * @brief Get scene the system is in.
		 * @return Scene the system is in.
		 */
		inline Scene* getScene() const
		{
			return m_scene;
		}

		/**
		 * @brief Called when a component is added to the system.
		 */
		virtual void onBegin();

		/**
		 * @brief Called once per tick.
		 * @param Delta time.
		 */
		virtual void onTick(float deltaTime);

		/**
		 * @brief Called once per tick after onTick().
		 * @param Delta time.
		 */
		virtual void onLateTick(float deltaTime);

		/**
		 * @brief Called once per tick, after onLateTick(), and before rendering.
		 * @param Delta time.
		 */
		virtual void onPreRender(float deltaTime);

		/**
		 * @brief Called when a component is removed from the system.
		 */
		virtual void onEnd();

		/**
		 * @brief Get a handle to the component being acted upon.
		 * @tparam Type of handle.
		 */
		template<class T>
		inline Handle<T> getComponent()
		{
			auto allocator = static_cast<ResourceAllocator<T>*>(m_components.get());
			return Handle<T>(allocator, m_componentHandle);
		}

	private:

		/** Scene the system is running in. */
		Scene* m_scene;

		/** Component allocator. */
		std::unique_ptr<ResourceAllocatorBase> m_components;

		/** Lambda function to run onTick(). */
		std::function<void(float)> m_runOnTick;

		/** Lambda function to run onTick(). */
		std::function<void(float)> m_runOnLateTick;

		/** Lambda function to run onTick(). */
		std::function<void(float)> m_runOnPreRender;

		/** Lambda function to destroy every component belonging to a given entity. */
		std::function<void(Entity)> m_destroyByEntity;

		/** Lambda function to destroy every component. */
		std::function<void()> m_destroyAllComponents;

		/** Lambda function to get the component belonging to a given entity. */
		std::function<ComponentBase*(Entity)> m_componentByEntity;

		/** ID of the component being acted upon. */
		size_t m_id;

		/** Handle of the component being acted upon. */
		size_t m_componentHandle;
	};
}