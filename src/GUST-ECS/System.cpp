#include "System.hpp"

namespace gust
{
	System::Iterator::Iterator(System* system, size_t pos) : m_system(system), m_handle(pos)
	{
		
	}

	System::Iterator::~Iterator()
	{

	}



	System::System(Scene* scene) : m_scene(scene), m_id(0), m_componentHandle(0)
	{

	}

	System::~System()
	{
		
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