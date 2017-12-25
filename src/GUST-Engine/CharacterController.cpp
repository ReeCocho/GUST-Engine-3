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
		m_targetRigidBody->activate();
		m_rigidBody->activate();

		auto transform = m_targetRigidBody->getWorldTransform();
		transform.setOrigin(transform.getOrigin() + btVector3(movement.x, movement.y, movement.z));
		m_targetRigidBody->setWorldTransform(transform);
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
		auto rot = collider->m_transform->getRotation();

		collider->m_lastPosition = pos;

		// Set transform
		btTransform transform = {};
		transform.setIdentity();
		transform.setOrigin({ pos.x, pos.y, pos.z });
		transform.setRotation({ rot.x, rot.y, rot.z, rot.w });

		// Create motion states
		collider->m_motionState = std::make_unique<btDefaultMotionState>(transform);
		collider->m_targetMotionState = std::make_unique<btDefaultMotionState>(transform);

		// Create sphere shape
		auto scale = collider->m_transform->getLocalScale();
		collider->m_radius = (scale.x + scale.z) / 2.0f;
		collider->m_height = scale.y;
		collider->m_shape = std::make_unique<btCapsuleShape>(collider->m_radius / 2.0f, collider->m_height - (2.0f * collider->m_radius));
		
		// Create rigid body
		{
			btRigidBody::btRigidBodyConstructionInfo info
			(
				1.0f,
				collider->m_motionState.get(),
				collider->m_shape.get()
			);

			// Create rigid body
			collider->m_rigidBody = std::make_unique<btRigidBody>(info);
		}
		
		// Create target rigid body
		{
			btRigidBody::btRigidBodyConstructionInfo info
			(
				1.0f,
				collider->m_targetMotionState.get(),
				nullptr
			);

			// Create rigid body
			collider->m_targetRigidBody = std::make_unique<btRigidBody>(info);
		}

		// Register bodies with dynamics world
		gust::physics.registerRigidBody(collider->m_rigidBody.get());
		gust::physics.registerRigidBody(collider->m_targetRigidBody.get());

		collider->m_rigidBody->setGravity(btVector3(0, 0, 0));
		collider->m_rigidBody->setSleepingThresholds(0.025f, 0.01f);
		collider->m_rigidBody->setAngularFactor(0);
		collider->m_targetRigidBody->setCollisionFlags(collider->m_targetRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		// collider->m_targetRigidBody->setActivationState(DISABLE_DEACTIVATION);

		// Create constraint
		{
			collider->m_constraint = std::make_unique<btGeneric6DofSpringConstraint>
			(
				*collider->m_rigidBody.get(), *collider->m_targetRigidBody.get(),
				btTransform(btQuaternion::getIdentity(), { 0.0f, 0.0f, 0.0f }),
				btTransform(btQuaternion::getIdentity(), { 0.0f, 0.0f, 0.0f }),
				true
			);

			// Removing any restrictions on the y-coordinate of the hanging box
			// by setting the lower limit above the upper one.
			collider->m_constraint->setLinearLowerLimit(btVector3(0.0f, 0.0f, 0.0f));
			collider->m_constraint->setLinearUpperLimit(btVector3(0.0f, 0.0f, 0.0f));

			collider->m_constraint->setAngularLowerLimit(btVector3(0, 0, 0));
			collider->m_constraint->setAngularUpperLimit(btVector3(0, 0, 0));

			collider->m_constraint->setBreakingImpulseThreshold(INFINITY);
		}

		// Register contraint
		gust::physics.registerConstraint(collider->m_constraint.get());
	}

	void CharacterControllerSystem::onLateTick(float deltaTime)
	{
		for (Handle<CharacterController> controller : *this)
		{
			// Check if the controller is grounded
			controller->m_grounded = false;
			if (gust::requestCollisionData(controller->m_rigidBody.get()).size() > 0)
				controller->m_grounded = true;

			auto newPos = controller->m_transform->getPosition();

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
				btTransform t = {};
				controller->m_motionState->getWorldTransform(t);
				auto pos = t.getOrigin();

				controller->m_transform->setPosition({ pos.x(), pos.y(), pos.z() });
			}

			// Set last transform
			controller->m_lastPosition = controller->m_transform->getPosition();
		}
	}

	void CharacterControllerSystem::onEnd()
	{
		auto collider = getComponent<CharacterController>();
		gust::physics.unregisterRigidBody(collider->m_rigidBody.get());
		gust::physics.unregisterRigidBody(collider->m_targetRigidBody.get());
		gust::physics.unregisterConstraint(collider->m_constraint.get());
	}
}