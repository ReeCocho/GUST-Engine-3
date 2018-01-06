#include <iostream>
#include "RigidBody.hpp"

/** RigidBody */
namespace gust
{
	RigidBody::RigidBody(Entity entity, Handle<RigidBody> handle) : Component<RigidBody>(entity, handle)
	{

	}

	RigidBody::~RigidBody()
	{

	}

	void RigidBody::setShapeNone()
	{
		m_shapeType = ShapeType::None;

		m_shape = std::make_shared<btEmptyShape>();
		m_rigidBody->setCollisionShape(m_shape.get());

		btVector3 inertia = { 0, 0, 0 };
		if (getMass() != 0)
			m_shape->calculateLocalInertia(getMass(), inertia);
		m_rigidBody->setMassProps(getMass(), inertia);
	}

	void RigidBody::setBoxShape(glm::vec3 dimensions)
	{
		m_shapeType = ShapeType::Box;

		m_shape = std::make_shared<btBoxShape>(btVector3(dimensions.x, dimensions.y, dimensions.z) / 2.0f);
		m_rigidBody->setCollisionShape(m_shape.get());

		btVector3 inertia = { 0, 0, 0 };
		if(getMass() != 0)
			m_shape->calculateLocalInertia(getMass(), inertia);
		m_rigidBody->setMassProps(getMass(), inertia);
	}

	void RigidBody::setSphereShape(float radius)
	{
		m_shapeType = ShapeType::Sphere;

		m_shape = std::make_shared<btSphereShape>(radius);
		m_rigidBody->setCollisionShape(m_shape.get());

		btVector3 inertia = { 0, 0, 0 };
		if (getMass() != 0)
			m_shape->calculateLocalInertia(getMass(), inertia);
		m_rigidBody->setMassProps(getMass(), inertia);

		m_rigidBody->activate();
	}

	void RigidBody::setCapsuleShape(float height, float radius)
	{
		m_shapeType = ShapeType::Capsule;

		m_shape = std::make_shared<btCapsuleShape>(radius, height);
		m_rigidBody->setCollisionShape(m_shape.get());

		btVector3 inertia = { 0, 0, 0 };
		if (getMass() != 0)
			m_shape->calculateLocalInertia(getMass(), inertia);
		m_rigidBody->setMassProps(getMass(), inertia);
	}



	RigidBodySystem::RigidBodySystem(Scene* scene) : System(scene)
	{
		initialize<RigidBody>();
	}

	RigidBodySystem::~RigidBodySystem()
	{

	}

	void RigidBodySystem::onBegin()
	{
		auto rigidBody = getComponent<RigidBody>();

		// Get transform component
		rigidBody->m_transform = rigidBody->getEntity().getComponent<Transform>();
		auto pos = rigidBody->m_transform->getPosition();
		auto rot = rigidBody->m_transform->getRotation();

		// Set physics transform
		btTransform transform = {};
		transform.setIdentity();
		transform.setOrigin({ pos.x, pos.y, pos.z });
		transform.setRotation({ rot.x, rot.y, rot.z, rot.w });

		// Create motion state
		rigidBody->m_motionState = std::make_shared<btDefaultMotionState>(transform);

		// Create shape
		rigidBody->m_shape = std::make_shared<btEmptyShape>();

		btRigidBody::btRigidBodyConstructionInfo info
		(
			1.0f,
			rigidBody->m_motionState.get(),
			rigidBody->m_shape.get()
		);

		// Create rigid body
		rigidBody->m_rigidBody = std::make_shared<btRigidBody>(info);

		// Register body with dynamics world
		gust::physics.registerRigidBody(rigidBody->m_rigidBody.get());
		rigidBody->m_rigidBody->setSleepingThresholds(0.025f, 0.01f);
	}

	void RigidBodySystem::onLateTick(float deltaTime)
	{
		for (Handle<RigidBody> rigidBody : *this)
		{
			// Get transform
			btTransform t = {};
			rigidBody->m_motionState->getWorldTransform(t);
			auto pos = t.getOrigin();
			auto rot = t.getRotation();

			rigidBody->m_transform->setPosition({ pos.x(), pos.y(), pos.z() });
			rigidBody->m_transform->setRotation({ rot.w(), rot.x(), rot.y(), rot.z() });
		}
	}

	void RigidBodySystem::onEnd()
	{
		auto rigidBody = getComponent<RigidBody>();

		gust::physics.unregisterRigidBody(rigidBody->m_rigidBody.get());
		rigidBody->m_motionState = nullptr;
		rigidBody->m_shape = nullptr;
		rigidBody->m_rigidBody = nullptr;
	}
}
