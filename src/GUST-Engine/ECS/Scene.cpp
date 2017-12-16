#include "Scene.hpp"

namespace gust
{
	void Scene::startup()
	{
		
	}

	void Scene::shutdown()
	{
		m_systems.clear();
	}

	void Scene::tick(float deltaTime)
	{
		// Call onTick()
		for (auto& system : m_systems)
			system->callOnTick(deltaTime);

		// Call onLateTick()
		for (auto& system : m_systems)
			system->callOnLateTick(deltaTime);

		// Call onPreRender()
		for (auto& system : m_systems)
			system->callOnPreRender(deltaTime);
	}
}