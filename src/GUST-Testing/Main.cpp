#include <iostream>
#include <Engine.hpp>
#include <Transform.hpp>
#include <MeshRenderer.hpp>
#include <Camera.hpp>
#include <Lights.hpp>
#include <Colliders.hpp>

class CameraController : public gust::Component<CameraController>
{
public:

	CameraController(gust::Entity entity, gust::Handle<CameraController> handle) : gust::Component<CameraController>(entity, handle)
	{

	}

	gust::Handle<gust::Transform> m_transform;

	bool m_enabled = true;
};

class CameraControllerSystem : public gust::System
{
public:

	CameraControllerSystem(gust::Scene* scene) : gust::System(scene)
	{
		initialize<CameraController>();
	}

	void onBegin() override
	{
		auto controller = getComponent<CameraController>();
		controller->m_transform = controller->getEntity().getComponent<gust::Transform>();
	}

	void onTick(float deltaTime) override
	{
		auto controller = getComponent<CameraController>();

		if (gust::input.getKeyDown(gust::KeyCode::M))
			controller->m_enabled = !controller->m_enabled;

		if (controller->m_enabled)
		{
			auto mouseDel = gust::input.getMouseDelta();

			float x = gust::input.getAxis("Horizontal");
			float y = gust::input.getAxis("Vertical");

			controller->m_transform->modPosition(controller->m_transform->getForward() * y * deltaTime);
			controller->m_transform->modPosition(controller->m_transform->getRight() * x * deltaTime);

			controller->m_transform->modEulerAngles(glm::vec3(mouseDel.y, mouseDel.x, 0) * 0.25f);
		}
	}
};

int main()
{
	// Initialize engine
	gust::startup("Test Game", 1280, 720);
	
	// Input stuff
	gust::input.registerAxis("Horizontal", { { gust::KeyCode::A, -1.0f }, { gust::KeyCode::D, 1.0f } });
	gust::input.registerAxis("Vertical", { { gust::KeyCode::S, -1.0f }, { gust::KeyCode::W, 1.0f } });
	
	// Add systems
	gust::scene.addSystem<gust::TransformSystem>();
	gust::scene.addSystem<gust::BoxColliderSystem>();
	gust::scene.addSystem<CameraControllerSystem>();
	gust::scene.addSystem<gust::PointLightSystem>();
	gust::scene.addSystem<gust::DirectionalLightSystem>();
	gust::scene.addSystem<gust::MeshRendererSystem>();
	gust::scene.addSystem<gust::CameraSystem>();
	
	auto shader = gust::resourceManager.createShader
	(
		"./Shaders/standard-vert.spv",
		"./Shaders/standard-frag.spv",
		sizeof(gust::EmptyVertexData),
		sizeof(gust::EmptyFragmentData),
		0,
		true,
		true
	);
	
	auto material = gust::resourceManager.createMaterial(shader);
	
	auto mesh = gust::resourceManager.createMesh("./Meshes/Cube.obj");

	// Create floor
	{
		auto entity = gust::Entity(&gust::scene);
	
		auto transform = entity.getComponent<gust::Transform>();
		transform->setLocalScale({ 16, 1, 16 });

		auto meshRenderer = entity.addComponent<gust::MeshRenderer>();
		meshRenderer->setMaterial(material);
		meshRenderer->setMesh(mesh);

		auto collider = entity.addComponent<gust::BoxCollider>();
		// collider->setMass(0);
		collider->setStatic(true);
		collider->setScale({ 16, 1, 16 });
	}
	
	// Create cube
	{
		auto entity = gust::Entity(&gust::scene);

		auto transform = entity.getComponent<gust::Transform>();
		transform->setPosition({ 0, 3, 4 });

		auto meshRenderer = entity.addComponent<gust::MeshRenderer>();
		meshRenderer->setMaterial(material);
		meshRenderer->setMesh(mesh);

		auto collider = entity.addComponent<gust::BoxCollider>();
	}

	// Create camera
	{
		auto entity = gust::Entity(&gust::scene);
	
		entity.addComponent<CameraController>();

		auto transform = entity.getComponent<gust::Transform>();
		transform->setPosition({ 0, 3, 0 });
	
		auto camera = entity.addComponent<gust::Camera>();
		gust::Camera::setMainCamera(camera);
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