#pragma once

/**
 * @file Colliders.hpp
 * @brief Colliders header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Math.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Transform.hpp"

/** Collider */
namespace gust
{
	/**
	 * @class Collider
	 * @brief Base class for other colliders.
	 */
	class Collider
	{
		friend class ColliderSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		Collider() = default;

		/**
		 * @brief Default constructor.
		 */
		~Collider() = default;

		/**
		 * @brief Get mass.
		 * @return Mass.
		 */
		inline float getMass() const
		{
			float m = m_rigidBody->getInvMass();
			return m == 0 ? 0 : 1.0f / m;
		}

		/**
		 * @brief Set mass.
		 * @param New mass.
		 * @return New mass.
		 */
		inline float setMass(float mass)
		{
			btVector3 inertia(0, 0, 0);
			m_rigidBody->activate(true);

			if (mass != 0)
				m_shape->calculateLocalInertia(mass, inertia);

			m_rigidBody->setMassProps(mass, inertia);

			return mass;
		}

		/**
		 * @brief Get linear velocity.
		 * @return Linear velocity.
		 */
		inline glm::vec3 getLinearVelocity() const
		{
			auto vel = m_rigidBody->getLinearVelocity();
			return glm::vec3(vel.x(), vel.y(), vel.z());
		}

		/**
		 * @brief Set linear velocity.
		 * @param New linear velocity.
		 * @return New linear velocity.
		 */
		inline glm::vec3 setLinearVelocity(glm::vec3 vel)
		{
			m_rigidBody->activate(true);
			m_rigidBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
			return vel;
		}

		/**
		 * @brief Get angular velocity.
		 * @return Angular velocity.
		 */
		inline glm::vec3 getAngularVelocity() const
		{
			auto vel = m_rigidBody->getAngularVelocity();
			return glm::vec3(vel.x(), vel.y(), vel.z());
		}

		/**
		 * @brief Set angular velocity.
		 * @param New angular velocity.
		 * @return New angular velocity.
		 */
		inline glm::vec3 setAngularVelocity(glm::vec3 vel)
		{
			m_rigidBody->activate(true);
			m_rigidBody->setAngularVelocity(btVector3(vel.x, vel.y, vel.z));
			return vel;
		}

		/**
		 * @brief Get friction.
		 * @return Friction.
		 */
		inline float getFriction() const
		{
			return m_rigidBody->getFriction();
		}

		/**
		 * @brief Set friction.
		 * @param New friction.
		 * @return New friction.
		 */
		inline float setFriction(float frict)
		{
			m_rigidBody->activate(true);
			m_rigidBody->setFriction(frict);
			return frict;
		}

		/**
		 * @brief Get rolling friction.
		 * @return Rolling friction.
		 */
		inline float getRollingFriction() const
		{
			return m_rigidBody->getRollingFriction();
		}

		/**
		 * @brief Set rolling friction.
		 * @param New rolling friction.
		 * @return New rolling friction.
		 */
		inline float setRollingFriction(float frict)
		{
			m_rigidBody->activate(true);
			m_rigidBody->setRollingFriction(frict);
			return frict;
		}

		/**
		 * @brief Get spinning friction.
		 * @return Spinning friction.
		 */
		inline float getSpinningFriction() const
		{
			return m_rigidBody->getSpinningFriction();
		}

		/**
		 * @brief Set spinning friction.
		 * @param New spinning friction.
		 * @return New spinning friction.
		 */
		inline float setSpinningFriction(float frict)
		{
			m_rigidBody->activate(true);
			m_rigidBody->setSpinningFriction(frict);
			return frict;
		}

		/**
		 * @brief Set rolling, spinning, and sliding friction.
		 * @param New frictions.
		 * @return New frictions.
		 */
		inline float setAllFrictions(float frict)
		{
			m_rigidBody->activate(true);
			m_rigidBody->setFriction(frict);
			m_rigidBody->setSpinningFriction(frict);
			m_rigidBody->setRollingFriction(frict);
			return frict;
		}

		/**
		 * @brief Get restitution.
		 * @return Restitution.
		 */
		inline float getRestitution() const
		{
			return m_rigidBody->getRestitution();
		}

		/**
		 * @brief Set restitution.
		 * @param New restitution.
		 * @return New restitution.
		 */
		inline float setRestitution(float rest)
		{
			m_rigidBody->activate(true);
			m_rigidBody->setRestitution(rest);
			return rest;
		}

		/**
		 * @brief Set if the rigid body is static.
		 * @param If the rigid body is static.
		 * @return If the rigid body is static.
		 * @note This will also set the mass to 0.
		 */
		inline bool setStatic(bool s)
		{
			if(s)
				setMass(0);

			auto flags = !(m_rigidBody->getCollisionFlags() ^ btCollisionObject::CF_STATIC_OBJECT);
			m_rigidBody->setCollisionFlags(flags | (s ? btCollisionObject::CF_STATIC_OBJECT : 0 ));

			return s;
		}

	private:

		/** Motion state. */
		std::unique_ptr<btMotionState> m_motionState = nullptr;

		/** Last recorded position. */
		glm::vec3 m_lastPosition = {};

		/** Last recorded rotation. */
		glm::quat m_lastRotation = {};

	protected:

		/** Game objects transform */
		Handle<Transform> m_transform = Handle<Transform>::nullHandle();

		/** Collision shape. */
		std::unique_ptr<btCollisionShape> m_shape = nullptr;

		/** Rigid body. */
		std::unique_ptr<btRigidBody> m_rigidBody = nullptr;
	};

	/**
	 * @class ColliderSystem
	 * @brief Base class for collider systems.
	 * @note This should not be added to a scene.
	 */
	class ColliderSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		ColliderSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~ColliderSystem();

		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		virtual void onBegin() override;

		/**
		 * @brief Called once per tick after onTick().
		 * @param Delta time.
		 */
		virtual void onLateTick(float deltaTime) override;

		/**
		 * @brief Called when a component is removed from the system.
		 */
		virtual void onEnd() override;

	protected:

		/**
		 * @brief Create the rigid body.
		 */
		void initRigidBody();

		/** Collider the system is working with. */
		Collider* m_collider = nullptr;

		/** Component the system is working with. */
		ComponentBase* m_component = nullptr;
	};
}

/** BoxCollider */
namespace gust
{
	/**
	 * @class BoxCollider
	 * @brief Allows an entity to have a box collider.
	 */
	class BoxCollider : public Component<BoxCollider>, public Collider
	{
		friend class BoxColliderSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		BoxCollider() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		BoxCollider(Entity entity, Handle<BoxCollider> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~BoxCollider();

		/**
		 * @brief Set box scale.
		 * @param New box scale.
		  *@return New box scale.
		 */
		inline glm::vec3 setScale(glm::vec3 scale)
		{
			m_scale = scale;

			btBoxShape* shape = static_cast<btBoxShape*>(m_shape.get());
			shape->setLocalScaling({ scale.x, scale.y, scale.z });

			return m_scale;
		}

		/**
		 * @brief Get box scale.
		 * @return Box scale.
		 */
		inline glm::vec3 getScale() const
		{
			return m_scale;
		}

	private:

		/** Box scale. */
		glm::vec3 m_scale = { 1, 1, 1 };
	};

	/**
	 * @class BoxColliderSystem
	 * @brief Implementation of a box collider.
	 */
	class BoxColliderSystem : public ColliderSystem
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		BoxColliderSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~BoxColliderSystem();

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