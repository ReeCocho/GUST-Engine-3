#include "Physics.hpp"

namespace gust
{
	void Physics::startup(glm::vec3 gravity)
	{
		// Create collision configuration
		m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();

		// Create dispatcher
		m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());

		// Create broadphase
		m_broadphase = std::make_unique<btDbvtBroadphase>();

		// Create constraint solver
		m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();

		// Create dynamics world
		m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>
		(
			m_dispatcher.get(),
			m_broadphase.get(),
			m_solver.get(),
			m_collisionConfig.get()
		);

		auto solver = m_dynamicsWorld->getSolverInfo();
		solver.m_numIterations = 50;

		// Set gravity vector
		m_dynamicsWorld->setGravity({ gravity.x, gravity.y, gravity.z });

		// Enable static on static collision reporting (Gets rid of a warning)
		m_dispatcher->setDispatcherFlags(m_dispatcher->getDispatcherFlags() | btCollisionDispatcher::CD_STATIC_STATIC_REPORTED);
	}

	void Physics::shutdown()
	{
		m_dynamicsWorld = nullptr;
		m_solver = nullptr;
		m_broadphase = nullptr;
		m_dispatcher = nullptr;
		m_collisionConfig = nullptr;
	}

	void Physics::step(float deltaTime)
	{
		// Do a step
		m_dynamicsWorld->stepSimulation(deltaTime, 50, 1 / 144.0f);
	}

	RaycastHitData Physics::linecast(glm::vec3 origin, glm::vec3 destination)
	{
		btCollisionWorld::ClosestRayResultCallback rayCallback
		(
			{ origin.x, origin.y, origin.z },
			{ destination.x, destination.y, destination.z }
		);

		// Perform the raycast
		m_dynamicsWorld->rayTest
		(
			{ origin.x, origin.y, origin.z },
			{ destination.x, destination.y, destination.z },
			rayCallback
		);

		// Raycast data
		RaycastHitData data = {};

		if (rayCallback.hasHit())
		{
			data.hit = true;

			data.point =
			{
				rayCallback.m_hitPointWorld.x(),
				rayCallback.m_hitPointWorld.y(),
				rayCallback.m_hitPointWorld.z()
			};

			data.normal =
			{
				rayCallback.m_hitNormalWorld.x(),
				rayCallback.m_hitNormalWorld.y(),
				rayCallback.m_hitNormalWorld.z()
			};
		}

		return data;
	}
}