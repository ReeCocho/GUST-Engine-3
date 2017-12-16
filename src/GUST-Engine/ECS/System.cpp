#include "System.hpp"

namespace gust
{
	SystemBase::SystemBase(Scene* scene, size_t id) : m_scene(scene), m_id(id)
	{

	}

	void SystemBase::callOnTick(float deltaTime)
	{

	}

	void SystemBase::callOnLateTick(float deltaTime)
	{

	}

	void SystemBase::callOnPreRender(float deltaTime)
	{

	}
}