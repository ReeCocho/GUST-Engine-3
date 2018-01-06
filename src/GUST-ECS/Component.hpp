#pragma once

/**
 * @file Component.hpp
 * @brief Component header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <type_traits>
#include "Entity.hpp"

namespace gust
{
	/**
	 * @class ComponentBase
	 * @brief An aspect of an entity.
	 */
	class ComponentBase
	{
	public:

		ComponentBase();

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to.
		 * @param Components ID.
		 */
		ComponentBase(Entity entity, size_t id);

		/**
		 * @brief Destructor.
		 */
		~ComponentBase();

		/**
		 * @brief Get entity the component is attached to.
		 * @return Entity the component is attached to.
		 */
		inline Entity getEntity() const
		{
			return m_entity;
		}

		/**
		 * @brief Get the scene the components entity is in.
		 * @return Scene the components entity is in.
		 */
		inline Scene* getScene() const
		{
			return m_entity.getScene();
		}

		/**
		 * @brief Get ID of the component
		 * @return ID of the component.
		 */
		inline size_t getID() const
		{
			return m_id;
		}

	private:

		/** Entity the component is attached to. */
		Entity m_entity;

		/** ID of the component. */
		size_t m_id;
	};

	/**
	 * @class TypeIDGenerator
	 * @brief A template class with a static member function to get IDs.
	 */
	template<class T>
	class TypeIDGenerator final
	{
	public:

		static const char id = 0;
	};

	/**
	 * @class TypeID
	 * @brief A template class to get ID.
	 */
	template<class T>
	class TypeID final
	{
	public:

		static_assert(std::is_base_of<ComponentBase, T>::value, "T must derive from Component");

		static constexpr inline size_t id()
		{
			// static size_t id = TypeIDGenerator::idCounter++;
			return reinterpret_cast<size_t>(&TypeIDGenerator<T>::id);
		}
	};

	template<class T>
	class Component : public ComponentBase
	{
	public:

		Component() : ComponentBase(Entity(nullptr, 0), 0)
		{

		}

		/**
		 * @brief Constructor.
		 * @param Entity the component belongs to.
		 * @param Components handle.
		 */
		Component(Entity entity, Handle<T> handle) : ComponentBase(entity, TypeID<T>::id())
		{
			m_handle = handle;
		}

		/**
		 * @brief Destructor.
		 */
		virtual ~Component()
		{

		}

		/**
		 * @brief Get the components handle.
		 * @return The components handle.
		 */
		inline Handle<T> getHandle()
		{
			return m_handle;
		}

	private:

		/** Handle to the component. */
		Handle<T> m_handle;
	};
}
