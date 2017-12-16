#include "Entity.hpp"

namespace gust
{
	Entity::Entity(Scene* scene) : m_scene(scene), m_handle(0)
	{

	}

	Entity::Entity(Scene* scene, size_t handle) : m_scene(scene), m_handle(handle)
	{

	}
}