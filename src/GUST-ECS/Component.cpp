#include "Component.hpp"

namespace gust
{
	ComponentBase::ComponentBase() : m_entity(Entity(nullptr, 0)), m_id(0)
	{

	}

	ComponentBase::ComponentBase(Entity entity, size_t id) : m_entity(entity), m_id(id)
	{

	}

	ComponentBase::~ComponentBase()
	{

	}
}