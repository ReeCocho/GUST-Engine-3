#include "Colliders.hpp"

/** Collider */
namespace gust
{
	std::vector<Handle<Collider>> Collider::colliders = {};



	ColliderSystem::ColliderSystem(Scene* scene) : System(scene)
	{

	}

	ColliderSystem::~ColliderSystem()
	{

	}

	void ColliderSystem::onBegin()
	{
		m_collider->m_transform = m_component->getEntity().getComponent<Transform>();
		auto pos = m_collider->m_transform->getPosition();
		auto rot = m_collider->m_transform->getRotation();

		// Set last transformation
		m_collider->m_lastPosition = pos;
		m_collider->m_lastRotation = rot;

		// Set transform
		btTransform transform = {};
		transform.setIdentity();
		transform.setOrigin({ pos.x, pos.y, pos.z });
		transform.setRotation({ rot.x, rot.y, rot.z, rot.w });

		// Create motion state
		m_collider->m_motionState = std::make_unique<btDefaultMotionState>(transform);
	}

	void ColliderSystem::onLateTick(float deltaTime)
	{
		auto newPos = m_collider->m_transform->getPosition();
		auto newRot = m_collider->m_transform->getRotation();

		// If the  transform was changed since the last time
		if (m_collider->m_lastPosition != newPos || m_collider->m_lastRotation != newRot)
		{
			btTransform t = {};
			t.setIdentity();
			t.setOrigin({ newPos.x, newPos.y, newPos.z });
			t.setRotation({ newRot.w, newRot.x, newRot.y, newRot.z });

			m_collider->m_motionState->setWorldTransform(t);
		}
		else
		{
			// Get transform
			btTransform t = {};
			m_collider->m_motionState->getWorldTransform(t);
			auto pos = t.getOrigin();
			auto rot = t.getRotation();

			m_collider->m_transform->setPosition({ pos.x(), pos.y(), pos.z() });
			m_collider->m_transform->setRotation({ rot.w(), rot.x(), rot.y(), rot.z() });
		}

		// Set last transform
		m_collider->m_lastPosition = m_collider->m_transform->getPosition();
		m_collider->m_lastRotation = m_collider->m_transform->getRotation();
	}

	void ColliderSystem::onEnd()
	{
		gust::physics.unregisterRigidBody(m_collider->m_rigidBody.get());
	}

	void ColliderSystem::initRigidBody()
	{
		btRigidBody::btRigidBodyConstructionInfo info
		(
			1.0f,
			m_collider->m_motionState.get(),
			m_collider->m_shape.get()
		);

		// Create rigid body
		m_collider->m_rigidBody = std::make_unique<btRigidBody>(info);

		// Register body with dynamics world
		gust::physics.registerRigidBody(m_collider->m_rigidBody.get());
		m_collider->m_rigidBody->setSleepingThresholds(0.025f, 0.01f);
		m_collider->m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
	}
}

/** BoxCollider */
namespace gust
{
	BoxCollider::BoxCollider(Entity entity, Handle<BoxCollider> handle) : Component<BoxCollider>(entity, handle)
	{

	}

	BoxCollider::~BoxCollider()
	{

	}

	BoxColliderSystem::BoxColliderSystem(Scene* scene) : ColliderSystem(scene)
	{
		initialize<BoxCollider>();
	}

	BoxColliderSystem::~BoxColliderSystem()
	{

	}

	void BoxColliderSystem::onBegin()
	{
		auto collider = getComponent<BoxCollider>();
		m_collider = static_cast<Collider*>(collider.get());
		m_component = static_cast<ComponentBase*>(collider.get());

		Collider::colliders.push_back(collider);

		ColliderSystem::onBegin();

		// Create box shape
		auto scale = collider->m_transform->getLocalScale() / 2.0f;
		collider->m_shape = std::make_unique<btBoxShape>(btVector3(scale.x, scale.y, scale.z));
		initRigidBody();
	}

	void BoxColliderSystem::onLateTick(float deltaTime)
	{
		for (Handle<BoxCollider> collider : *this)
		{
			m_collider = static_cast<Collider*>(collider.get());
			m_component = static_cast<ComponentBase*>(collider.get());

			ColliderSystem::onLateTick(deltaTime);
		}
	}

	void BoxColliderSystem::onEnd()
	{
		auto collider = getComponent<BoxCollider>();
		m_collider = static_cast<Collider*>(collider.get());
		m_component = static_cast<ComponentBase*>(collider.get());

		Collider::colliders.erase(std::remove(Collider::colliders.begin(), Collider::colliders.end(), Handle<Collider>(collider)), Collider::colliders.end());
		ColliderSystem::onEnd();
	}
}

/** SphereCollider */
namespace gust
{
	SphereCollider::SphereCollider(Entity entity, Handle<SphereCollider> handle) : Component<SphereCollider>(entity, handle)
	{

	}

	SphereCollider::~SphereCollider()
	{

	}

	SphereColliderSystem::SphereColliderSystem(Scene* scene) : ColliderSystem(scene)
	{
		initialize<SphereCollider>();
	}

	SphereColliderSystem::~SphereColliderSystem()
	{

	}

	void SphereColliderSystem::onBegin()
	{
		auto collider = getComponent<SphereCollider>();
		m_collider = static_cast<Collider*>(collider.get());
		m_component = static_cast<ComponentBase*>(collider.get());

		Collider::colliders.push_back(collider);

		ColliderSystem::onBegin();

		// Create sphere shape
		auto scale = collider->m_transform->getLocalScale();
		collider->m_shape = std::make_unique<btSphereShape>((glm::abs(scale.x) + glm::abs(scale.y) + glm::abs(scale.z)) / 6.0f);
		initRigidBody();
	}

	void SphereColliderSystem::onLateTick(float deltaTime)
	{
		for (Handle<SphereCollider> collider : *this)
		{
			m_collider = static_cast<Collider*>(collider.get());
			m_component = static_cast<ComponentBase*>(collider.get());

			ColliderSystem::onLateTick(deltaTime);
		}
	}

	void SphereColliderSystem::onEnd()
	{
		auto collider = getComponent<SphereCollider>();
		m_collider = static_cast<Collider*>(collider.get());
		m_component = static_cast<ComponentBase*>(collider.get());

		Collider::colliders.erase(std::remove(Collider::colliders.begin(), Collider::colliders.end(), Handle<Collider>(collider)), Collider::colliders.end());
		ColliderSystem::onEnd();
	}
}