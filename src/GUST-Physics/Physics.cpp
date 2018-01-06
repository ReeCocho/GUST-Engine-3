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

		m_dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

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

		// Clear collision data
		std::queue<PhysicsCollisionData> emptyQueue = std::queue<PhysicsCollisionData>();
		std::swap(m_collisionData, emptyQueue);

		// Get number of manifolds and loop over them
		int numManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();
		for (int i = 0; i < numManifolds; ++i)
		{
			// Get objects colliding
			btPersistentManifold* contactManifold = m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
			const btCollisionObject* obA = contactManifold->getBody0();
			const btCollisionObject* obB = contactManifold->getBody1();

			// Get number of contacts and loop over them
			int numContacts = contactManifold->getNumContacts();
			for (int j = 0; j < numContacts; ++j)
			{
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				if (pt.getDistance() <= 0.0f)
				{
					const btVector3& ptA = pt.getPositionWorldOnA();
					const btVector3& ptB = pt.getPositionWorldOnB();
					const btVector3& normalOnB = pt.m_normalWorldOnB;

					btRigidBody* rbA = nullptr;
					btRigidBody* rbB = nullptr;

					// Find bodies
					for (auto rigidBody : m_rigidBodies)
					{
						if (rbA && rbB)
							break;

						if (obA == rigidBody)
							rbA = rigidBody;

						if (obB == rigidBody)
							rbB = rigidBody;
					}

					PhysicsCollisionData data = {};
					data.point = { ptA.x(), ptA.y(), ptA.z() };
					data.normal = { normalOnB.x(), normalOnB.y(), normalOnB.z() };
					data.touched = rbB;
					data.touching = rbA;

					if (pt.getDistance() < 0)
						data.penetration = -pt.getDistance();

					// Add collision data
					m_collisionData.emplace(data);
				}
			}
		}
	}

	bool Physics::pollPhysicsCollisionData(PhysicsCollisionData& data)
	{
		if (m_collisionData.size() > 0)
		{
			// Get data
			data = m_collisionData.front();
			m_collisionData.pop();
			return true;
		}

		return false;
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
