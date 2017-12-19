#include "Transform.hpp"

namespace gust
{
	Transform::Transform(Entity entity, Handle<Transform> handle) : Component<Transform>(entity, handle)
	{

	}

	Transform::~Transform()
	{

	}

	glm::vec3 Transform::setPosition(glm::vec3 value)
	{
		m_position = value;

		if (m_parent == Handle<Transform>::nullHandle())
			m_localPosition = m_position;
		else
		{
			glm::vec4 newPos = glm::inverse(m_parent->m_unscaledModelMatrix) * glm::vec4(m_position, 1.0);
			m_localPosition = glm::vec3(newPos.x, newPos.y, newPos.z);
		}

		generateModelMatrix();
		updateChildren();

		return m_position;
	}

	glm::vec3 Transform::setLocalPosition(glm::vec3 value)
	{
		m_localPosition = value;

		if (m_parent == Handle<Transform>::nullHandle())
			m_position = value;
		else
		{
			glm::vec4 newPos = m_parent->m_unscaledModelMatrix * glm::vec4(m_localPosition, 1.0);
			m_position = glm::vec3(newPos.x, newPos.y, newPos.z);
		}

		generateModelMatrix();
		updateChildren();

		return m_localPosition;
	}

	glm::quat Transform::setRotation(glm::quat value)
	{
		m_rotation = value;
		m_eulerAngles = glm::degrees(glm::eulerAngles(value));

		if (m_parent == Handle<Transform>::nullHandle())
		{
			m_localRotation = m_rotation;
			m_localEulerAngles = m_eulerAngles;
		}
		else
		{
			m_localRotation = inverse(m_parent->m_rotation) * m_rotation;
			m_localEulerAngles = glm::degrees(glm::eulerAngles(m_localRotation));
		}

		generateModelMatrix();
		updateChildren();

		return m_rotation;
	}

	glm::quat Transform::setLocalRotation(glm::quat value)
	{
		m_localRotation = value;
		m_localEulerAngles = glm::degrees(glm::eulerAngles(value));

		if (m_parent == Handle<Transform>::nullHandle())
		{
			m_rotation = m_localRotation;
			m_eulerAngles = m_localEulerAngles;
		}
		else
		{
			m_rotation = m_parent->m_rotation * m_localRotation;
			m_eulerAngles = glm::degrees(glm::eulerAngles(m_rotation));
		}

		generateModelMatrix();
		updateChildren();

		return m_localRotation;
	}

	glm::vec3 Transform::setEulerAngles(glm::vec3 value)
	{
		value.x = std::fmod(value.x, 360.0f);
		value.y = std::fmod(value.y, 360.0f);
		value.z = std::fmod(value.z, 360.0f);

		m_eulerAngles = value;
		m_rotation = glm::quat(glm::radians(value));
		
		if (m_parent == Handle<Transform>::nullHandle())
		{
			m_localRotation = m_rotation;
			m_localEulerAngles = m_eulerAngles;
		}
		else
		{
			m_localRotation = glm::inverse(m_parent->m_rotation) * m_rotation;
			m_localEulerAngles = glm::degrees(glm::eulerAngles(m_localRotation));
		}
		
		generateModelMatrix();
		updateChildren();

		return m_eulerAngles;
	}

	glm::vec3 Transform::setLocalEulerAngles(glm::vec3 value)
	{
		value.x = std::fmod(value.x, 360.0f);
		value.y = std::fmod(value.y, 360.0f);
		value.z = std::fmod(value.z, 360.0f);

		m_localEulerAngles = value;
		m_localRotation = glm::quat(glm::radians(value));

		if (m_parent == Handle<Transform>::nullHandle())
		{
			m_eulerAngles = m_localEulerAngles;
			m_rotation = m_localRotation;
		}
		else
		{
			m_rotation = m_parent->m_rotation * m_localRotation;
			m_eulerAngles = glm::degrees(glm::eulerAngles(m_rotation));
		}

		generateModelMatrix();
		updateChildren();

		return m_localEulerAngles;
	}

	glm::vec3 Transform::setLocalScale(glm::vec3 value)
	{
		m_localScale = value;
		generateModelMatrix();
		updateChildren();
		return m_localScale;
	}

	Handle<Transform> Transform::setParent(Handle<Transform> parent)
	{
		// Remove self from parents child list
		if (m_parent != Handle<Transform>::nullHandle())
			parent->m_children.erase(std::remove(parent->m_children.begin(), parent->m_children.end(), getHandle()), parent->m_children.end());

		// Set parent
		m_parent = parent;

		// Add self to new parents children list
		if (m_parent != Handle<Transform>::nullHandle())
			parent->m_children.push_back(getHandle());

		// Update local values and model matrix
		setPosition(getPosition());
		setRotation(getRotation());

		return m_parent;
	}

	void Transform::generateModelMatrix()
	{
		m_modelMatrix = {};
		m_unscaledModelMatrix = {};

		// Translation matrix
		m_modelMatrix = glm::translate(m_modelMatrix, m_localPosition);

		// Rotation matrix
		m_modelMatrix *= glm::mat4_cast(m_localRotation);
		m_unscaledModelMatrix = m_modelMatrix;

		// Scale matrix
		m_modelMatrix = glm::scale(m_modelMatrix, m_localScale);

		// Get parent matrix if we have one
		if (m_parent != Handle<Transform>::nullHandle())
		{
			m_modelMatrix = m_parent->m_unscaledModelMatrix * m_modelMatrix;
			m_unscaledModelMatrix = m_parent->m_unscaledModelMatrix * m_unscaledModelMatrix;
		}

		// Update childrens model matrix
		for (auto child : m_children)
			child->generateModelMatrix();
	}

	void Transform::updateChildren()
	{
		for (auto child : m_children)
		{
			// Local position to global position
			child->setLocalPosition(child->m_localPosition);

			// Local euler angles to global euler angles
			child->setLocalRotation(child->m_localRotation);

			// child->generateModelMatrix();
			// child->updateChildren();
		}
	}



	TransformSystem::TransformSystem(Scene* scene) : System(scene)
	{
		initialize<Transform>();
	}

	TransformSystem::~TransformSystem()
	{

	}

	void TransformSystem::onBegin()
	{
		getComponent<Transform>()->generateModelMatrix();
	}
}