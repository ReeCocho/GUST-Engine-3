#include <iostream>
#include "CharacterController.hpp"

namespace gust
{
	CharacterController::CharacterController(Entity entity, Handle<CharacterController> handle) : Component<CharacterController>(entity, handle)
	{

	}

	CharacterController::~CharacterController()
	{

	}

	void CharacterController::move(glm::vec3 movement)
	{
		m_rigidBody->activate();
		
		btTransform t = m_rigidBody->getWorldTransform();
		t.setOrigin(t.getOrigin() + btVector3(movement.x, movement.y, movement.z));

		m_rigidBody->setWorldTransform(t);

		// m_rigidBody->setLinearVelocity(btVector3(movement.x, movement.y, movement.z));
	}



	CharacterControllerSystem::CharacterControllerSystem(Scene* scene) : System(scene)
	{
		initialize<CharacterController>();
	}

	CharacterControllerSystem::~CharacterControllerSystem()
	{

	}

	void CharacterControllerSystem::onBegin()
	{
		auto controller = getComponent<CharacterController>();

		controller->m_transform = controller->getEntity().getComponent<Transform>();
		auto pos = controller->m_transform->getPosition();

		controller->m_lastPosition = pos;

		// Set transform
		btTransform transform = {};
		transform.setIdentity();
		transform.setOrigin({ pos.x, pos.y, pos.z });

		// Create motion states
		controller->m_motionState = std::make_unique<btDefaultMotionState>(transform);

		// Create sphere shape
		auto scale = controller->m_transform->getLocalScale();
		controller->m_radius = (scale.x + scale.z) / 2.0f;
		controller->m_height = scale.y;
		controller->m_shape = std::make_unique<btCapsuleShape>
		(
			controller->m_radius / 2.0f, 
			controller->m_height - (2.0f * controller->m_radius)
		);

		// Create rigid body
		{
			btRigidBody::btRigidBodyConstructionInfo info
			(
				1.0f,
				controller->m_motionState.get(),
				controller->m_shape.get()
			);

			// Create rigid body
			controller->m_rigidBody = std::make_unique<btRigidBody>(info);

			controller->m_rigidBody->setFriction(0);
			controller->m_rigidBody->setRestitution(0);
			controller->m_rigidBody->setGravity(btVector3(0, 0, 0));
			controller->m_rigidBody->setAngularFactor(0);
			controller->m_rigidBody->setCollisionFlags(controller->m_rigidBody->getCollisionFlags() | btRigidBody::CF_KINEMATIC_OBJECT);
		}

		// Register bodies with dynamics world
		gust::physics.registerRigidBody(controller->m_rigidBody.get());
	}

	void CharacterControllerSystem::onLateTick(float deltaTime)
	{
		for (Handle<CharacterController> controller : *this)
		{
			auto newPos = controller->m_transform->getPosition();
			controller->m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));

			// If the  transform was changed since the last time
			if (controller->m_lastPosition != newPos)
			{
				btTransform t = {};
				t.setIdentity();
				t.setOrigin({ newPos.x, newPos.y, newPos.z });

				controller->m_rigidBody->setWorldTransform(t);
			}
			else
			{
				// Get transform
				btTransform t = controller->m_rigidBody->getWorldTransform();
				auto pos = t.getOrigin();

				controller->m_transform->setPosition({ pos.x(), pos.y(), pos.z() });
			}

			// Set last transform
			controller->m_lastPosition = controller->m_transform->getPosition();

			// Check if the controller is grounded
			{
				controller->m_grounded = false;

				float cosSliding = glm::cos(glm::radians(controller->m_slidingAngle));

				const auto& collisionData = gust::requestCollisionData(controller->m_rigidBody.get());
				for (auto data : collisionData)
				{
					std::cout << "T\n";
					btTransform t = controller->m_rigidBody->getWorldTransform();
					t.setOrigin(t.getOrigin() - (btVector3(data.normal.x, data.normal.y, data.normal.z) * data.penetration));
					auto pos = t.getOrigin();

					if (data.point.y < controller->m_lastPosition.y - (controller->m_height / 2.0f) && glm::dot(glm::vec3(0, -1, 0), -data.normal) < cosSliding)
					{
						controller->m_grounded = true;
						break;
					}
				}
			}

			// // Clamp us to the floor
			// {
			// 	glm::vec3 origin = controller->m_transform->getPosition();
			// 	origin.y -= controller->m_height / 2.0f;
			// 	origin.y -= controller->m_radius;
			// 
			// 	RaycastHitData hitData = gust::physics.raycast(origin, glm::vec3(0, -1, 0), controller->m_floorClampDistance);
			// 
			// 	if (hitData.hit)
			// 	{
			// 		btTransform t = controller->m_rigidBody->getWorldTransform();
			// 		t.setOrigin(t.getOrigin() - btVector3(0, ((origin.y - hitData.point.y) / 2.0f) * deltaTime, 0));
			// 		auto pos = t.getOrigin();
			// 
			// 		controller->m_rigidBody->setWorldTransform(t);
			// 		controller->m_transform->setPosition({ pos.x(), pos.y(), pos.z() });
			// 	}
			// }
		}
	}

	void CharacterControllerSystem::onEnd()
	{
		auto collider = getComponent<CharacterController>();
		gust::physics.unregisterRigidBody(collider->m_rigidBody.get());
	}
}