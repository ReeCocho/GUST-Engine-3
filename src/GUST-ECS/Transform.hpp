#pragma once

/**
 * @file Transform.hpp
 * @brief Transform header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Math.hpp"
#include "Scene.hpp"

namespace gust
{
	/**
	 * @class Transform
	 * @brief Allows an entiy to be represented in world space.
	 */
	class Transform : public Component<Transform>
	{
		friend class TransformSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		Transform() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		Transform(Entity entity, Handle<Transform> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~Transform();

		/**
		 * @brief Get the transforms position.
		 * @return Position.
		 */
		inline glm::vec3 getPosition() const
		{
			return m_position;
		}

		/**
		 * @brief Get the transform local position.
		 * @return Local position.
		 */
		inline glm::vec3 getLocalPosition() const
		{
			return m_localPosition;
		}

		/**
		 * @brief Get the transforms rotation.
		 * @return Rotation.
		 */
		inline glm::quat getRotation() const
		{
			return m_rotation;
		}

		/**
		 * @brief Get the transforms local rotation.
		 * @return Local rotation.
		 */
		inline glm::quat getLocalRotation() const
		{
			return m_localRotation;
		}

		/**
		 * @brief Get the transforms euler angles.
		 * @return Euler angles.
		 */
		inline glm::vec3 getEulerAngles() const
		{
			return m_eulerAngles;
		}

		/**
		 * @brief Get the transforms local euler angles.
		 * @return Local euler angles.
		 */
		inline glm::vec3 getLocalEulerAngles() const
		{
			return m_localEulerAngles;
		}

		/**
		 * @brief Get the transforms local scale.
		 * @return Local scale.
		 */
		inline glm::vec3 getLocalScale() const
		{
			return m_localScale;
		}

		/**
		 * @brief Get the transforms model matrix.
		 * @return Model matrix.
		 */
		inline glm::mat4 getModelMatrix() const
		{
			return m_modelMatrix;
		}

		/**
		 * @brief Get a forward vector realative to the transform.
		 * @return Forward vector.
		 */
		inline glm::vec3 getForward() const
		{
			auto mat = glm::mat4_cast(m_rotation);
			auto front = glm::vec3(mat[2].x, mat[2].y, mat[2].z);

			return front;
		}

		/**
		 * @brief Get a up vector realative to the transform.
		 * @return Up vector.
		 */
		inline glm::vec3 getUp() const
		{
			auto mat = glm::mat4_cast(m_rotation);
			auto up = glm::vec3(mat[1].x, mat[1].y, mat[1].z);

			return up;
		}

		/**
		 * @brief Get a right vector realative to the transform.
		 * @return Right vector.
		 */
		inline glm::vec3 getRight() const
		{
			auto mat = glm::mat4_cast(m_rotation);
			auto right = glm::vec3(mat[0].x, mat[0].y, mat[0].z);

			return right;
		}

		/**
		 * @brief Get the transforms parent.
		 * @return Transforms parent.
		 */
		inline Handle<Transform> getParent() const
		{
			return m_parent;
		}

		/**
		 * @brief Get the transforms Nth child.
		 * @return Nth child.
		 * @note Returns nullptr if n is out of bounds.
		 */
		inline Handle<Transform> getChild(size_t n) const
		{
			if (n < 0 || n >= m_children.size())
				return Handle<Transform>(nullptr, 0);

			return m_children[n];
		}

		/**
		 * @brief Get the number of children the transform contains.
		 * @return Number of children.
		 */
		inline size_t childCount() const
		{
			return m_children.size();
		}



		/**
		 * @brief Set the transforms position.
		 * @param New position.
		 * @return New position.
		 */
		glm::vec3 setPosition(glm::vec3 value);

		/**
		 * @brief Set the transforms local position.
		 * @param New local position.
		 * @return New local position.
		 */
		glm::vec3 setLocalPosition(glm::vec3 value);

		/**
		 * @brief Set the transforms rotation.
		 * @param New rotation.
		 * @return New rotation.
		 */
		glm::quat setRotation(glm::quat value);

		/**
		 * @brief Set the transforms local rotation.
		 * @param New local rotation.
		 * @return New local rotation.
		 */
		glm::quat setLocalRotation(glm::quat value);

		/**
		 * @brief Set the transforms euler angles.
		 * @param New euler angles.
		 * @return New euler angles.
		 */
		glm::vec3 setEulerAngles(glm::vec3 value);

		/**
		 * @brief Set the transforms local euler angles.
		 * @param New local euler angles.
		 * @return New local euler angles.
		 */
		glm::vec3 setLocalEulerAngles(glm::vec3 value);

		/**
		 * @brief Set the transforms local scale.
		 * @param New local scale.
		 * @return New local scale.
		 */
		glm::vec3 setLocalScale(glm::vec3 value);

		/**
		 * @brief Set the transforms parent.
		 * @param New parent transform.
		 * @return New parent transform.
		 */
		Handle<Transform> setParent(Handle<Transform> parent);


		/**
		 * @brief Add the position dispalcement to the old position.
		 * @param Position displacement.
		 * @return New position.
		 */
		inline glm::vec3 modPosition(glm::vec3 value)
		{
			return setPosition(m_position + value);
		}

		/**
		 * @brief Add the local position dispalcement to the old local position.
		 * @param Local position displacement.
		 * @return New local position.
		 */
		inline glm::vec3 modLocalPosition(glm::vec3 value)
		{
			return setLocalPosition(m_localPosition + value);
		}

		/**
		 * @brief Multiply the rotation dispalcement by the old rotation.
		 * @param Rotation displacement.
		 * @return New rotation.
		 */
		inline glm::quat modRotation(glm::quat value)
		{
			return setRotation(m_rotation * value);
		}

		/**
		 * @brief Multiply the local rotation dispalcement by the old local rotation.
		 * @param Local rotation displacement.
		 * @return New local rotation.
		 */
		inline glm::quat modLocalRotation(glm::quat value)
		{
			return setLocalRotation(m_localRotation * value);
		}

		/**
		 * @brief Add the euler angles dispalcement to the old euler angles.
		 * @param Euler angles displacement.
		 * @return New euler angles.
		 */
		inline glm::vec3 modEulerAngles(glm::vec3 value)
		{
			return setEulerAngles(m_eulerAngles + value);
		}

		/**
		 * @brief Add the local euler angles dispalcement to the old local euler angles.
		 * @param Local euler angles displacement.
		 * @return New local euler angles.
		 */
		inline glm::vec3 modLocalEulerAngles(glm::vec3 value)
		{
			return setLocalEulerAngles(m_localEulerAngles + value);
		}

		/**
		 * @brief Add the local scale dispalcement to the old local scale.
		 * @param Local scale displacement.
		 * @return New local scale.
		 */
		inline glm::vec3 modLocalScale(glm::vec3 value)
		{
			return setLocalScale(m_localScale + value);
		}

		/**
		 * @brief Add a new child to the transform.
		 * @param Transform component of the new child.
		 */
		inline Handle<Transform> addChild(Handle<Transform> obj)
		{
			obj->setParent(getHandle());
			return obj;
		}

	private:

		/**
		 * @brief Generate a new model matrix.
		 */
		void generateModelMatrix();

		/**
		 * @brief Update the transforms children.
		 */
		void updateChildren();

		/** Postion. */
		glm::vec3 m_position = {};

		/** Local position. */
		glm::vec3 m_localPosition = {};

		/** Euler angles. */
		glm::vec3 m_eulerAngles = {};

		/** Local euler angles. */
		glm::vec3 m_localEulerAngles = {};

		/** Local scale. */
		glm::vec3 m_localScale = { 1, 1, 1 };

		/** Rotation. */
		glm::quat m_rotation = {};

		/** Local rotation. */
		glm::quat m_localRotation = {};

		/** Model matrix. */
		glm::mat4 m_modelMatrix = {};

		/** Model matrix without scale applied (Used for hierarchy.) */
		glm::mat4 m_unscaledModelMatrix = {};

		/** Parent. */
		Handle<Transform> m_parent = Handle<Transform>::nullHandle();

		/** Children. */
		std::vector<Handle<Transform>> m_children = {};
	};



	/**
	 * @class TransformSystem
	 * @brief Tranform system.
	 */
	class TransformSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		TransformSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~TransformSystem();

		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		void onBegin() override;

		/**
		 * @brief Called when a component is removed from the system.
		 */
		void onEnd() override;
	};
}