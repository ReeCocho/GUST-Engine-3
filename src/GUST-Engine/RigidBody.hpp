#pragma once

/**
 * @file RigidBody.hpp
 * @brief Rigid body header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Math.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Transform.hpp"

/** RigidBody */
namespace gust
{
	/**
	 * @enum ShapeType
	 * @brief Type of collision shape.
	 */
	enum class ShapeType
	{
		None = 0,
		Box = 1,
		Sphere = 2,
		Capsule = 3
	};

	/**
	 * @class RigidBody
	 * @brief Performs physics calculations on an entity.
	 */
	class RigidBody : public Component<RigidBody>
	{
		friend class RigidBodySystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		RigidBody() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		RigidBody(Entity entity, Handle<RigidBody> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~RigidBody();



		/**
		 * @brief Get shape type.
		 * @return Shape type.
		 */
		inline ShapeType getShapeType() const
		{
			return m_shapeType;
		}

		/**
		 * @brief Remove shape.
		 */
		void setShapeNone();

		/**
		 * @brief Set box shape.
		 * @param Dimensions.
		 */
		void setBoxShape(glm::vec3 dimensions);

		/**
		 * @brief Set sphere shape.
		 * @param Radius.
		 */
		void setSphereShape(float radius);

		/**
		 * @brief Set capsule shape.
		 * @param Height.
		 * @param Radius.
		 */
		void setCapsuleShape(float height, float radius);

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
			if(s) setMass(0);

			auto flags = !(m_rigidBody->getCollisionFlags() ^ btCollisionObject::CF_STATIC_OBJECT);
			m_rigidBody->setCollisionFlags(flags | (s ? btCollisionObject::CF_STATIC_OBJECT : 0 ));

			return s;
		}

	private:

		/** Game objects transform */
		Handle<Transform> m_transform = Handle<Transform>::nullHandle();

		/** Shape type. */
		ShapeType m_shapeType = ShapeType::None;

		/** Collision shape. */
		std::shared_ptr<btCollisionShape> m_shape = nullptr;

		/** Motion state. */
		std::shared_ptr<btMotionState> m_motionState = nullptr;

		/** Rigid body. */
		std::shared_ptr<btRigidBody> m_rigidBody = nullptr;
	};

	/**
	 * @class RigidBodySystem
	 * @brief Implementation of a rigid body.
	 */
	class RigidBodySystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		RigidBodySystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~RigidBodySystem();

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
	
	/**
	 * @struct CollisionData
	 * @brief Information about a collision.
	 */
	struct CollisionData
	{
		/** Collider tocuhed. */
		Handle<RigidBody> touched = {};

		/** Collider touching. */
		Handle<RigidBody> touching = {};

		/** Contact point in world space. */
		glm::vec3 point = {};

		/** Normal on touched object. */
		glm::vec3 normal = {};

		/** Penetration. */
		float penetration = 0;
	};

	/**
	 * @class CollisionCallback
	 * @brief Callback manager for colliders.
	 */
	class CollisionCallback
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Collider the callback is associated with.
		 * @param Callback.
		 */
		CollisionCallback(Handle<Collider> collider, std::function<void(CollisionData)> callback);

		/**
		 * @brief Destructor.
		 */
		~CollisionCallback();

		/**
		 * @brief Invoke the callback.
		 * @param Collision data.
		 */
		void invoke(CollisionData data);

		/**
		 * @brief Get collider.
		 * @return Collider.
		 */
		inline Handle<RigidBody> getRigidBody()
		{
			return m_rigidBody;
		}

	private:

		/** Collider. */
		Handle<RigidBody> m_rigidBody = Handle<RigidBody>::nullHandle();

		/** Callback. */
		std::function<void(CollisionData)> m_callback;
	};
}