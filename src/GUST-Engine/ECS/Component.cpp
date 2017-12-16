#include "Component.hpp"

namespace gust
{
	ComponentBase::ComponentBase(Entity entity, size_t id) : m_entity(entity), m_id(id)
	{

	}

	ComponentBase::~ComponentBase()
	{

	}
}