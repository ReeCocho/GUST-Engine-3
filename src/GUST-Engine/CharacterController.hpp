#pragma once

/**
 * @file CharacterController.hpp
 * @brief Character controller header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Math.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Transform.hpp"

namespace gust
{
	/**
	 * @class CharacterController
	 * @brief Allows an entity to act as a character.
	 */
	class CharacterController : public Component<CharacterController>
	{
		friend class CharacterControllerSystem;

	public:

		/**
		* @brief Default constructor.
		*/
		CharacterController() = default;

		/**
		* @brief Constructor.
		* @param Entity the component is attached to
		* @param Component handle
		*/
		CharacterController(Entity entity, Handle<CharacterController> handle);

		/**
		* @brief Destructor.
		* @see Component::~Component
		*/
		~CharacterController();

		/**
		 * @brief Get collision shape.
		 * @return Collision shape.
		 */
		inline btCollisionShape* getCollisionShape()
		{
			return m_shape.get();
		}

		/**
		 * @brief Set character radius.
		 * @param New character radius.
		 * @return New character radius.
		 */
		inline float setRadius(float radius)
		{
			m_radius = radius;
			
			m_shape = std::make_unique<btCapsuleShape>(m_radius, m_height);
			m_rigidBody->setCollisionShape(m_shape.get());

			return m_radius;
		}

		/**
		 * @brief Get character radius
		 * @return Character radius
		 */
		inline float getRadius() const
		{
			return m_radius;
		}

		/**
		 * @brief Set character height.
		 * @param New character height.
		 * @return New character height.
		 */
		inline float setHeight(float height)
		{
			m_height = height;

			m_shape = std::make_unique<btCapsuleShape>(m_radius, m_height);
			m_rigidBody->setCollisionShape(m_shape.get());

			return m_height;
		}

		/**
		 * @brief Get character height.
		 * @return Character height.
		 */
		inline float getHeight() const
		{
			return m_height;
		}

		/**
		 * @brief Get if the controller is grounded.
		 * @return If the controller is grounded.
		 */
		inline bool isGrounded() const
		{
			return m_grounded;
		}

		/**
		 * @brief Move the controller.
		 * @param Movement delta.
		 */
		void move(glm::vec3 movement);

	protected:

		/** Game objects transform */
		Handle<Transform> m_transform = Handle<Transform>::nullHandle();

		/** Motion state. */
		std::unique_ptr<btMotionState> m_motionState = nullptr;

		/** Target motion state. */
		std::unique_ptr<btMotionState> m_targetMotionState = nullptr;

		/** Collision shape. */
		std::unique_ptr<btCollisionShape> m_shape = nullptr;

		/** Rigid body. */
		std::unique_ptr<btRigidBody> m_rigidBody = nullptr;

		/** Target body. */
		std::unique_ptr<btRigidBody> m_targetRigidBody = nullptr;

		/** Target constraint. */
		std::unique_ptr<btGeneric6DofConstraint> m_constraint = nullptr;

		/** Last recorded position. */
		glm::vec3 m_lastPosition = {};

		/** Is the controller grounded. */
		bool m_grounded = false;

		/** Sphere radius */
		float m_radius = 0.5f;

		/** Capsule height. */
		float m_height = 2.0f;
	};

	/**
	 * @class CharacterControllerSystem
	 * @brief Implementation of a character controller.
	 */
	class CharacterControllerSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		CharacterControllerSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~CharacterControllerSystem();

		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		void onBegin() override;

		/**
		 * @brief Called once per tick after onTick().
		 * @param Delta time.
		 */
		void onLateTick(float deltaTime) override;

		/**
		 * @brief Called when a component is removed from the system.
		 */
		void onEnd() override;
	};
}