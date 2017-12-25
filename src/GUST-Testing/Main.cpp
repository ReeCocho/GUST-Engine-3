#include <iostream>
#include <Engine.hpp>
#include <Transform.hpp>
#include <MeshRenderer.hpp>
#include <Camera.hpp>
#include <Lights.hpp>
#include <Colliders.hpp>
#include <CharacterController.hpp>

class SpinningObject : public gust::Component<SpinningObject>
{
public:

	SpinningObject(gust::Entity entity, gust::Handle<SpinningObject> handle) : gust::Component<SpinningObject>(entity, handle)
	{

	}

	gust::Handle<gust::Transform> m_transform;
};

class SpinningObjectSystem : public gust::System
{
public:

	SpinningObjectSystem(gust::Scene* scene) : gust::System(scene)
	{
		initialize<SpinningObject>();
	}

	void onBegin() override
	{
		auto component = getComponent<SpinningObject>();
		component->m_transform = component->getEntity().getComponent<gust::Transform>();
	}

	void onTick(float deltaTime) override
	{

	}
};

class Player : public gust::Component<Player>
{
public:

	Player(gust::Entity entity, gust::Handle<Player> handle) : gust::Component<Player>(entity, handle)
	{

	}

	gust::Handle<gust::Transform> m_transform;

	gust::Handle<gust::CharacterController> m_controller;

	gust::Handle<gust::Transform> m_cameraTransform;

	float m_camRot = 0;

	bool m_enabled = true;
};

class PlayerSystem : public gust::System
{
public:

	PlayerSystem(gust::Scene* scene) : gust::System(scene)
	{
		initialize<Player>();
	}

	void onBegin() override
	{
		auto player = getComponent<Player>();
		player->m_transform = player->getEntity().getComponent<gust::Transform>();
		player->m_cameraTransform = player->m_transform->getChild(0);
		player->m_controller = player->getEntity().getComponent<gust::CharacterController>();
	}

	void onTick(float deltaTime) override
	{
		for (gust::Handle<Player> player : *this)
		{
			float x = gust::input.getAxis("Horizontal");
			float y = gust::input.getAxis("Vertical");

			if (x != 0 || y != 0)
			{
				auto forward = player->m_cameraTransform->getForward();
				forward.y = 0;
				forward = glm::normalize(forward);

				auto right = player->m_cameraTransform->getRight();
				right.y = 0;
				right = glm::normalize(right);

				glm::vec3 movementVector = forward * y;
				movementVector += right * x;
				movementVector = glm::normalize(movementVector);

				player->m_controller->move(movementVector * deltaTime);
			}

			if (gust::input.getKeyDown(gust::KeyCode::M))
				player->m_enabled = !player->m_enabled;

			gust::input.setLockedMouse(player->m_enabled);

			if (player->m_enabled)
			{
				auto mouseDel = gust::input.getMouseDelta();
				mouseDel.x /= static_cast<float>(gust::graphics.getWidth()) / 2.0f;
				mouseDel.y /= static_cast<float>(gust::graphics.getHeight()) / 2.0f;
				player->m_transform->modEulerAngles(glm::vec3(0, mouseDel.x, 0) * 50.0f);

				player->m_camRot += (mouseDel.y * 50.0f);

				if (player->m_camRot < -89.0f)
					player->m_camRot = -89.0f;

				if (player->m_camRot > 89.0f)
					player->m_camRot = 89.0f;

				player->m_cameraTransform->setLocalEulerAngles(glm::vec3(player->m_camRot, 0, 0));
			}
		}
	}
};

struct TestData
{
	glm::vec2 uv = {};
};

int main()
{
	// Initialize engine
	gust::startup("Test Game", 1280, 720);
	
	// Input stuff
	gust::input.registerAxis("Horizontal", { { gust::KeyCode::A, -1.0f }, { gust::KeyCode::D, 1.0f } });
	gust::input.registerAxis("Vertical", { { gust::KeyCode::S, -1.0f }, { gust::KeyCode::W, 1.0f } });
	
	// Graphics setup
	gust::renderer.setAmbientColor({ 0.9f, 0.9f, 1 });
	gust::renderer.setAmbientIntensity(0.5f);

	// Add systems
	{
		// Core systems
		gust::scene.addSystem<gust::TransformSystem>();

		// Custom systems
		gust::scene.addSystem<SpinningObjectSystem>();
		gust::scene.addSystem<PlayerSystem>();

		// Physics systems
		gust::scene.addSystem<gust::CharacterControllerSystem>();
		gust::scene.addSystem<gust::BoxColliderSystem>();
		gust::scene.addSystem<gust::SphereColliderSystem>();
		gust::scene.addSystem<gust::CapsuleColliderSystem>();

		// Rendering systems
		gust::scene.addSystem<gust::PointLightSystem>();
		gust::scene.addSystem<gust::DirectionalLightSystem>();
		gust::scene.addSystem<gust::SpotLightSystem>();
		gust::scene.addSystem<gust::MeshRendererSystem>();
		gust::scene.addSystem<gust::CameraSystem>();
	}

	// Create shaders
	auto shader = gust::resourceManager.createShader
	(
		"./Shaders/standard-vert.spv",
		"./Shaders/standard-frag.spv",
		sizeof(gust::EmptyVertexData),
		sizeof(TestData),
		2,
		true,
		true
	);

	// Create textures
	auto floor_n = gust::resourceManager.createTexture("./Textures/BrickFloor_n.png", vk::Filter::eLinear);
	auto floor = gust::resourceManager.createTexture("./Textures/BrickFloor.jpg", vk::Filter::eLinear);
	auto pom = gust::resourceManager.createTexture("./Textures/CutePom.png", vk::Filter::eLinear);
	auto gabe = gust::resourceManager.createTexture("./Textures/Gabe.jpg", vk::Filter::eLinear);

	TestData data = {};
	data.uv = { 6.0f, 6.0f };

	// Create materials
	auto floor_mat = gust::resourceManager.createMaterial(shader);
	floor_mat->setTexture(floor, 0);
	floor_mat->setTexture(floor_n, 1);
	floor_mat->setFragmentData<TestData>(data);
		
	data.uv = { 1, 1 };

	auto pom_mat = gust::resourceManager.createMaterial(shader);
	pom_mat->setTexture(gabe, 0);
	pom_mat->setTexture(floor_n, 1);
	pom_mat->setFragmentData<TestData>(data);

	// Create meshes
	auto cube_mesh = gust::resourceManager.createMesh("./Meshes/Cube.obj");
	auto sphere_mesh = gust::resourceManager.createMesh("./Meshes/Sphere.obj");

	// Create floor
	{
		auto entity = gust::Entity(&gust::scene);
	
		auto transform = entity.getComponent<gust::Transform>();
		transform->setLocalScale({ 16, 1, 16 });

		auto meshRenderer = entity.addComponent<gust::MeshRenderer>();
		meshRenderer->setMaterial(floor_mat);
		meshRenderer->setMesh(cube_mesh);

		auto collider = entity.addComponent<gust::BoxCollider>();
		collider->setStatic(true);
	}
	
	// Create cube
	{
		auto entity = gust::Entity(&gust::scene);

		auto transform = entity.getComponent<gust::Transform>();
		transform->setPosition({ 0, 3, 4 });

		auto meshRenderer = entity.addComponent<gust::MeshRenderer>();
		meshRenderer->setMaterial(pom_mat);
		meshRenderer->setMesh(cube_mesh);

		auto collider = entity.addComponent<gust::BoxCollider>();
	}

	// Create pillar
	{
		auto entity = gust::Entity(&gust::scene);

		auto transform = entity.getComponent<gust::Transform>();
		transform->setPosition({ -5, 3, 4 });
		transform->setLocalScale({ 1, 3, 1 });
		transform->setLocalEulerAngles({ 90, 0, 0 });

		auto meshRenderer = entity.addComponent<gust::MeshRenderer>();
		meshRenderer->setMaterial(pom_mat);
		meshRenderer->setMesh(cube_mesh);

		auto collider = entity.addComponent<gust::CapsuleCollider>();
	}

	// Create sphere
	{
		auto entity = gust::Entity(&gust::scene);

		auto transform = entity.getComponent<gust::Transform>();
		transform->setPosition({ 3, 6, 4 });
		transform->setLocalScale({ 2, 2, 2 });

		auto meshRenderer = entity.addComponent<gust::MeshRenderer>();
		meshRenderer->setMaterial(pom_mat);
		meshRenderer->setMesh(sphere_mesh);

		auto collider = entity.addComponent<gust::SphereCollider>();
		
		entity.addComponent<SpinningObject>();
	}

	// Create player
	{
		auto entity = gust::Entity(&gust::scene);

		auto transform = entity.getComponent<gust::Transform>();
		transform->setPosition({ -3, 3, -3 });

		auto controller = entity.addComponent<gust::CharacterController>();
		controller->setHeight(1.0f);
		controller->setRadius(0.5f);

		// Create camera
		{
			auto entity2 = gust::Entity(&gust::scene);

			auto transform2 = entity2.getComponent<gust::Transform>();
			transform2->setParent(transform);
			transform2->setLocalPosition({ 0, 1, 0 });

			auto camera = entity2.addComponent<gust::Camera>();
			gust::Camera::setMainCamera(camera);
		}

		// Create mesh
		{
			auto entity2 = gust::Entity(&gust::scene);
			
			auto transform2 = entity2.getComponent<gust::Transform>();
			transform2->setParent(transform);
			transform2->setLocalPosition({ 0, 0, 0 });
			transform2->setLocalScale({ 1, 2, 1 });

			auto meshRenderer2 = entity2.addComponent<gust::MeshRenderer>();
			meshRenderer2->setMaterial(pom_mat);
			meshRenderer2->setMesh(cube_mesh);
		}

		auto player = entity.addComponent<Player>();
	}
	
	// Create light
	{
		auto entity = gust::Entity(&gust::scene);
	
		auto transform = entity.getComponent<gust::Transform>();
		// transform->setPosition({ 0, 3, 0 });
		transform->setEulerAngles({ 45, 60, 0 });
	
		auto light = entity.addComponent<gust::DirectionalLight>();
	}
	
	gust::simulate();
	gust::shutdown();

	std::cin.get();
	return 0;
}