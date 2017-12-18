#include "Scene.hpp"
#include "../Components/Transform.hpp"

namespace gust
{
	void Scene::startup()
	{
		
	}

	void Scene::shutdown()
	{
		for (auto& system : m_systems)
			system->m_destroyAllComponents();

		m_systems.clear();
	}

	size_t Scene::create()
	{
		size_t handle = 0;

		// Check to see if we have a free handle
		if (m_freeEntityHandles.size() != 0)
		{
			handle = m_freeEntityHandles.front();
			m_freeEntityHandles.pop();
		}
		else
			handle = ++m_entityHandleCounter;

		// Add transform component
		addComponent<Transform>(Entity(this, handle));

		return handle;
	}

	void Scene::destroyMarkedEntities()
	{
		// Loop over every system
		for (auto entityHandle : m_markedEntities)
		{
			auto components = getComponentsOfEntity(Entity(this, entityHandle));

			size_t oldSize = m_markedComponents.size();
			m_markedComponents.resize(oldSize + components.size());

			for (size_t i = 0; i < components.size(); ++i)
				m_markedComponents[oldSize + i] = components[i];
		}

		m_markedEntities.clear();
	}

	void Scene::destroyMarkedComponents()
	{
		for (auto component : m_markedComponents)
		{
			System* system = getSystemOfType(component->getID());
			system->m_destroyByEntity(component->getEntity());
		}

		m_markedComponents.clear();
	}

	void Scene::destroy(size_t handle)
	{
		m_markedEntities.push_back(handle);
	}

	void Scene::tick(float deltaTime)
	{
		// Destroy stuff if needed
		destroyMarkedEntities();
		destroyMarkedComponents();

		// Call onTick()
		for (auto& system : m_systems)
			system->m_runOnTick(deltaTime);

		// Call onLateTick()
		for (auto& system : m_systems)
			system->m_runOnLateTick(deltaTime);

		// Call onPreRender()
		for (auto& system : m_systems)
			system->m_runOnPreRender(deltaTime);
	}

	System* Scene::getSystemOfType(size_t id)
	{
		// Loop over every system checking what component it works with
		for (auto& system : m_systems)
			if (system->getID() == id)
				return system.get();

		return nullptr;
	}

	std::vector<ComponentBase*> Scene::getComponentsOfEntity(Entity entity)
	{
		std::vector<ComponentBase*> components(m_systems.size());

		size_t index = 0;
		for (auto& system : m_systems)
		{
			ComponentBase* component = system->m_componentByEntity(entity);

			if (component)
			{
				components[index] = component;
				++index;
			}
		}

		components.resize(index);
		return components;
	}
}