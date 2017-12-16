#pragma once

/**
 * @file System.hpp
 * @brief System header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <memory>
#include "../Utilities/Allocators.hpp"
#include "Component.hpp"

namespace gust
{
	/**
	 * @class SystemBase
	 * @brief Base class for systems.
	 */
	class SystemBase
	{
		friend class Scene;

	public:

		/**
		 * @brief Default constructor.
		 */
		SystemBase() = default;

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 * @param ID of the component we are acting upon.
		 */
		SystemBase(Scene* scene, size_t id);

		/**
		 * @brief Default destructor.
		 */
		virtual ~SystemBase() = default;

		/**
		 * @brief Call onTick() for every component.
		 * @param Delta time.
		 * @note Used internally. Do not call.
		 */
		virtual void callOnTick(float deltaTime);

		/**
		 * @brief Call onLateTick() for every component.
		 * @param Delta time.
		 * @note Used internally. Do not call.
		 */
		virtual void callOnLateTick(float deltaTime);

		/**
		 * @brief Call onPreRender() for every component.
		 * @param Delta time.
		 * @note Used internally. Do not call.
		 */
		virtual void callOnPreRender(float deltaTime);

		/**
		 * @brief Get ID of the component being acted upon.
		 * @return ID of the component being acted upon.
		 */
		size_t getID() const
		{
			return m_id;
		}

		/**
		 * @brief Get scene the system is in.
		 * @return Scene the system is in.
		 */
		Scene* getScene()
		{
			return m_scene;
		}

	private:

		/** ID of the component being acted upon. */
		size_t m_id;

		/** Scene the system is running in. */
		Scene* m_scene;
	};

	/**
	 * @class System
	 * @brief Manages components and running game code.
	 */
	template<class T>
	class System : public SystemBase
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		System() = default;

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		System(Scene* scene) : SystemBase(scene, TypeID<T>::id())
		{
			m_components = ResourceAllocator<T>(50, alignof(T));
		}

		/**
		 * @brief Default destructor.
		 */
		virtual ~System() = default;



		/**
		 * @brief Call onTick() for every component.
		 * @param Delta time.
		 * @note Used internally. Do not call.
		 */
		void callOnTick(float deltaTime) override
		{
			for(size_t i = 0; i < m_components.getMaxResourceCount(); i++)
				if (m_components.isAllocated(i))
					onTick(Handle<T>(&m_components, i), deltaTime);
		}

		/**
		 * @brief Call onLateTick() for every component.
		 * @param Delta time.
		 * @note Used internally. Do not call.
		 */
		void callOnLateTick(float deltaTime) override
		{
			for (size_t i = 0; i < m_components.getMaxResourceCount(); i++)
				if (m_components.isAllocated(i))
					onLateTick(Handle<T>(&m_components, i), deltaTime);
		}

		/**
		 * @brief Call onPreRender() for every component.
		 * @param Delta time.
		 * @note Used internally. Do not call.
		 */
		void callOnPreRender(float deltaTime) override
		{
			for (size_t i = 0; i < m_components.getMaxResourceCount(); i++)
				if (m_components.isAllocated(i))
					onPreRender(Handle<T>(&m_components, i), deltaTime);
		}



		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		virtual void onBegin(Handle<T> component)
		{

		}

		/**
		 * @brief Called once per tick.
		 * @param Component to act upon.
		 * @param Delta time.
		 */
		virtual void onTick(Handle<T> component, float deltaTime)
		{

		}
		
		/**
		 * @brief Called once per tick after onTick().
		 * @param Component to act upon.
		 * @param Delta time.
		 */
		virtual void onLateTick(Handle<T> component, float deltaTime)
		{

		}

		/**
		 * @brief Called once per tick, after onLateTick(), and before rendering.
		 * @param Component to act upon.
		 * @param Delta time.
		 */
		virtual void onPreRender(Handle<T> component, float deltaTime)
		{

		}

		/**
		 * @brief Called when a component is removed from the system.
		 * @param Component to act upon.
		 */
		virtual void onEnd(Handle<T> component)
		{

		}

	private:

		/** Component allocator. */
		ResourceAllocator<T> m_components;
	};
}