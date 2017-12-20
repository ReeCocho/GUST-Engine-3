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
#include <btBulletDynamicsCommon.h>

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
		}

		/**
		 * @brief Unregister a rigid body.
		 * @param Rigid body to unregister.
		 */
		inline void unregisterRigidBody(btRigidBody* body)
		{
			m_dynamicsWorld->removeRigidBody(body);
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
	};
}