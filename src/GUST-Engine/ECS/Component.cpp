#include "Component.hpp"

namespace gust
{
	size_t TypeIDGenerator::idCounter = 0;



	ComponentBase::ComponentBase(Entity entity, size_t id) : m_entity(entity), m_id(id)
	{

	}

	ComponentBase::~ComponentBase()
	{

	}
}