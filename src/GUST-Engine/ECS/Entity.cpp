#include "Scene.hpp"
#include "Entity.hpp"

namespace gust
{
	Entity::Entity(Scene* scene) : m_scene(scene)
	{
		m_handle = m_scene->create();
	}

	Entity::Entity(Scene* scene, size_t handle) : m_scene(scene), m_handle(handle)
	{

	}
}