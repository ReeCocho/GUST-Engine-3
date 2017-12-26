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
		m_rigidBody->setLinearVelocity(btVector3(movement.x, movement.y, movement.z));
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
		auto collider = getComponent<CharacterController>();

		collider->m_transform = collider->getEntity().getComponent<Transform>();
		auto pos = collider->m_transform->getPosition();

		collider->m_lastPosition = pos;

		// Set transform
		btTransform transform = {};
		transform.setIdentity();
		transform.setOrigin({ pos.x, pos.y, pos.z });

		// Create motion states
		collider->m_motionState = std::make_unique<btDefaultMotionState>(transform);

		// Create sphere shape
		auto scale = collider->m_transform->getLocalScale();
		collider->m_radius = (scale.x + scale.z) / 2.0f;
		collider->m_height = scale.y;
		collider->m_shape = std::make_unique<btCapsuleShape>(collider->m_radius / 2.0f, collider->m_height - (2.0f * collider->m_radius));

		// Create rigid body
		{
			btRigidBody::btRigidBodyConstructionInfo info
			(
				0.01f,
				collider->m_motionState.get(),
				collider->m_shape.get()
			);

			// Create rigid body
			collider->m_rigidBody = std::make_unique<btRigidBody>(info);

			collider->m_rigidBody->setGravity(btVector3(0, 0, 0));
			collider->m_rigidBody->setAngularFactor(0);
		}

		// Register bodies with dynamics world
		gust::physics.registerRigidBody(collider->m_rigidBody.get());
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
			controller->m_grounded = false;

			float cosSliding = glm::cos(glm::radians(controller->m_slidingAngle));

			const auto& collisionData = gust::requestCollisionData(controller->m_rigidBody.get());
			for (auto data : collisionData)
				if (data.point.y < controller->m_lastPosition.y - (controller->m_height / 2.0f) && glm::dot(glm::vec3(0, -1, 0), -data.normal) < cosSliding)
				{
					controller->m_grounded = true;
					break;
				}
		}
	}

	void CharacterControllerSystem::onEnd()
	{
		auto collider = getComponent<CharacterController>();
		gust::physics.unregisterRigidBody(collider->m_rigidBody.get());
	}
}