#pragma once

/**
 * @file System.hpp
 * @brief System header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "../Utilities/Allocators.hpp"

namespace gust
{
	class Scene;

	/**
	 * @class SystemBase
	 * @brief Base class for systems.
	 */
	class SystemBase
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		SystemBase() = default;

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		SystemBase(Scene* scene);

		/**
		 * @brief Default destructor.
		 */
		virtual ~SystemBase() = default;

		/**
		 * @brief Get scene the system is in.
		 * @return Scene the system is in.
		 */
		Scene* getScene()
		{
			return m_scene;
		}

	private:

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
		System(Scene* scene) : SystemBase(scene)
		{
			m_components = ResourceAllocator<T>(50, alignof(T));
		}

		/**
		 * @brief Default destructor.
		 */
		~System() = default;

		/**
		 * @brief Called when a component is added to the system.
		 */
		virtual void onBegin()
		{

		}

		/**
		 * @brief Called once per tick.
		 * @param Delta time.
		 */
		virtual void onTick(float deltaTime)
		{

		}
		
		/**
		 * @brief Called once per tick after onTick().
		 * @param Delta time.
		 */
		virtual void onLateTick(float deltaTime)
		{

		}

		/**
		 * @brief Called once per tick, after onLateTick(), and before rendering.
		 * @param Delta time.
		 */
		virtual void onPreRender(float deltaTime)
		{

		}

		/**
		 * @brief Called when a component is removed from the system.
		 */
		virtual void onEnd()
		{

		}

	private:

		/** Component allocator. */
		ResourceAllocator<T> m_components;
	};
}