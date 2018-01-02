#pragma once

/**
 * @file System.hpp
 * @brief System header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <functional>
#include <memory>
#include <Allocators.hpp>
#include <Math.hpp>

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
		friend class SystemIterator;

	public:

		/**
		 * @class Iterator
		 * @brief Class used to iterate over components in a system.
		 */
		class Iterator
		{
		public:

			/**
			 * @brief Default constructor.
			 */
			Iterator() = default;

			/**
			 * @brief Constructor.
			 * @param System to iterate over.
			 * @param Starting position for the handle.
			 */
			Iterator(System* system, size_t pos);

			/**
			 * @brief Default destructor.
			 */
			~Iterator();

			/**
			 * @brief Unequivilence operator.
			 * @param Left side.
			 * @param Right side.
			 * @return If the iterators are equal.
			 */
			bool operator!=(const Iterator& other) const
			{
				return m_handle != other.m_handle;
			}

			/**
			 * @brief Increment operator.
			 * @return Self.
			 */
			Iterator& operator++()
			{
				++m_handle;

				while (m_handle != m_system->m_components->getMaxResourceCount() && !m_system->m_components->isAllocated(m_handle))
					++m_handle;

				m_system->m_componentHandle = m_handle;
				return *this;
			}

			/**
			 * @brief Indirection operator.
			 * @tparam Component type.
			 * @return Active component.
			 */
			Handle<ComponentBase> operator*()
			{
				return Handle<ComponentBase>(m_system->m_components.get(), m_handle);
			}

		private:

			/** System to iterate over. */
			System* m_system;

			/** Handle. */
			size_t m_handle;
		};



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

		/**
		 * @brief Get iterator at the beginning of the component list.
		 * @return Iterator at the beginning of the component list.
		 */
		Iterator begin()
		{
			size_t index = 0;
			for (index; index <= m_components->getMaxResourceCount(); ++index)
			{
				if (index == m_components->getMaxResourceCount() || m_components->isAllocated(index))
					break;
			}

			return Iterator(this, index);
		}

		/**
		 * @brief Get iterator at the end of the component list.
		 * @return Iterator at the end of the component list.
		 */
		Iterator end()
		{
			return Iterator(this, m_components->getMaxResourceCount());
		}

	private:

		/** Scene the system is running in. */
		Scene* m_scene;

		/** Component allocator. */
		std::unique_ptr<ResourceAllocatorBase> m_components;

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