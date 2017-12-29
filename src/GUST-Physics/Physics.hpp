#pragma once

/**
 * @file Physics.hpp
 * @brief Physics header file.
 * @author Connor J. Bramham (ReeCocho)
 */

 /** Defines. */
#define GUST_PHYSICS_STEP_RATE (1.0f/60.0f)

 /** Includes. */
#include <glm\glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision\CollisionDispatch\btGhostObject.h>

namespace gust
{
	/**
	 * @struct RaycastHitData
	 * @brief Holds information about a raycast hit.
	 */
	struct RaycastHitData
	{
		/** Did the raycast hit? */
		bool hit = false;

		/** Point of contact. */
		glm::vec3 point = {};

		/** Normal to the hit surface. */
		glm::vec3 normal = {};
	};

	/**
	 * @struct PhysicsCollisionData
	 * @brief Data retrieved during a collision.
	 */
	struct PhysicsCollisionData
	{
		/** Collider tocuhed. */
		btCollisionObject* touched = nullptr;

		/** Collider touching. */
		btCollisionObject* touching = nullptr;

		/** Contact point in world space. */
		glm::vec3 point = {};

		/** Normal on touched object. */
		glm::vec3 normal = {};

		/** Penetration. */
		float penetration = 0.0f;
	};



	/**
	 * @class Physics
	 * @brief Physics engine powered by Bullet Physics.
	 */
	class Physics
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Physics() = default;

		/**
		 * @brief Default destructor.
		 */
		~Physics() = default;

		/**
		 * @brief Initialize physics engine.
		 * @param Gravity vector.
		 */
		void startup(glm::vec3 gravity);

		/**
		 * @brief Shutdown physics engine.
		 */
		void shutdown();

		/**
		 * @brief Perform a step in the physics simulation.
		 * @param Time since last frame.
		 */
		void step(float deltaTime);

		/**
		 * @brief Register a rigid body.
		 * @param Rigid body to register.
		 */
		inline void registerRigidBody(btRigidBody* body)
		{
			m_dynamicsWorld->addRigidBody(body);
			m_rigidBodies.push_back(body);
		}

		/**
		 * @brief Unregister a rigid body.
		 * @param Rigid body to unregister.
		 */
		inline void unregisterRigidBody(btRigidBody* body)
		{
			m_dynamicsWorld->removeRigidBody(body);
			m_rigidBodies.erase(std::remove(m_rigidBodies.begin(), m_rigidBodies.end(), body), m_rigidBodies.end());
		}

		/**
		 * @brief Register a collision object.
		 * @param Collision object to register.
		 */
		inline void registerCollisionObject(btCollisionObject* obj)
		{
			m_dynamicsWorld->addCollisionObject(obj);
		}

		/**
		 * @brief Unregister a collision object.
		 * @param Collision object to unregister.
		 */
		inline void unregisterCollisionObject(btCollisionObject* obj)
		{
			m_dynamicsWorld->addCollisionObject(obj);
		}

		/**
		 * @brief Register a constraint.
		 * @param Constriant to register.
		 */
		inline void registerConstraint(btTypedConstraint* constraint)
		{
			m_dynamicsWorld->addConstraint(constraint);
		}

		/**
		 * @brief Unregister a constraint.
		 * @param Constriant to unregister.
		 */
		inline void unregisterConstraint(btTypedConstraint* constraint)
		{
			m_dynamicsWorld->removeConstraint(constraint);
		}

		/**
		 * @brief Perform a linecast.
		 * @brief Origin.
		 * @brief Destination.
		 * @return Hit data.
		 */
		RaycastHitData linecast(glm::vec3 origin, glm::vec3 destination);

		/**
		 * @brief Perform a raycast.
		 * @brief Origin.
		 * @brief Direction vector (Should be normalized).
		 * @brief Magnitude
		 * @return Hit data.
		 */
		inline RaycastHitData raycast(glm::vec3 origin, glm::vec3 direction, float magnitude)
		{
			return linecast(origin, origin + (direction * magnitude));
		}

		/**
		 * @brief Poll collision data.
		 * @param Collision data object to store the data inside of.
		 * @return If we polled any data.
		 */
		bool pollPhysicsCollisionData(PhysicsCollisionData& data);

		/**
		 * @brief Get dynamics world.
		 * @return Dynamics world.
		 */
		inline btDiscreteDynamicsWorld* getDynamicsWorld()
		{
			return m_dynamicsWorld.get();
		}

	private:

		/** Collision configuration. */
		std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;

		/** Collision dispatcher. */
		std::unique_ptr<btCollisionDispatcher> m_dispatcher;

		/** Broadphase. */
		std::unique_ptr<btBroadphaseInterface> m_broadphase;

		/** Constraint solver. */
		std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;

		/** Dynamics world. */
		std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

		/** Rigid bodies registered with the engine. */
		std::vector<btRigidBody*> m_rigidBodies = {};

		/** Collision data from the last step. */
		std::queue<PhysicsCollisionData> m_collisionData = {};
	};
}