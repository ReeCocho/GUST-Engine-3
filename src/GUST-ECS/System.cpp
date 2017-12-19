#include "System.hpp"

namespace gust
{
	System::System(Scene* scene) : m_scene(scene), m_id(0), m_componentHandle(0)
	{

	}

	System::~System()
	{
		// m_destroyAllComponents();
	}

	void System::onBegin()
	{

	}

	void System::onTick(float deltaTime)
	{

	}

	void System::onLateTick(float deltaTime)
	{

	}

	void System::onPreRender(float deltaTime)
	{

	}

	void System::onEnd()
	{
		
	}
}