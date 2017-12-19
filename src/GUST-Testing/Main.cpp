#include <iostream>
#include <GUST-Engine\Engine.hpp>
#include <GUST-Engine\Components\Transform.hpp>
#include <GUST-Engine\Components\MeshRenderer.hpp>
#include <GUST-Engine\Components\Camera.hpp>

class TestComponent : public gust::Component<TestComponent>
{
public:

	TestComponent(gust::Entity entity, gust::Handle<TestComponent> handle) : gust::Component<TestComponent>(entity, handle)
	{

	}

	gust::Handle<gust::Transform> m_transform;
};

class TestComponentSystem : public gust::System
{
public:

	TestComponentSystem(gust::Scene* scene) : gust::System(scene)
	{
		initialize<TestComponent>();
	}

	void onBegin() override
	{
		auto component = getComponent<TestComponent>();
		component->m_transform = component->getEntity().getComponent<gust::Transform>();
	}

	void onTick(float deltaTime) override
	{
		auto component = getComponent<TestComponent>();
		component->m_transform->modEulerAngles({ 0, deltaTime * 60.0f, 0 });
	}
};

int main()
{
	gust::Engine::get().startup("Test Game", 1280, 720);
	
	auto& scene = gust::Engine::get().scene;
	scene.addSystem<gust::TransformSystem>();
	scene.addSystem<TestComponentSystem>();
	scene.addSystem<gust::MeshRendererSystem>();
	scene.addSystem<gust::CameraSystem>();
	
	auto shader = gust::Engine::get().resourceManager.createShader
	(
		"./Shaders/standard-vert.spv",
		"./Shaders/standard-frag.spv",
		sizeof(gust::EmptyVertexData),
		sizeof(gust::EmptyFragmentData),
		0,
		true
	);
	
	auto material = gust::Engine::get().resourceManager.createMaterial(shader);
	
	auto mesh = gust::Engine::get().resourceManager.createMesh("./Meshes/Cube.obj");
	
	// Create cube
	{
		auto entity = gust::Entity(&scene);
		// entity.addComponent<TestComponent>();
	
		auto meshRenderer = entity.addComponent<gust::MeshRenderer>();
		//meshRenderer->setMaterial(material);
		meshRenderer->setMesh(mesh);
	}
	
	// Create camera
	{
		auto entity = gust::Entity(&scene);
	
		auto transform = entity.getComponent<gust::Transform>();
		transform->setPosition({ 1, 0, -2 });
	
		auto camera = entity.addComponent<gust::Camera>();
		gust::Camera::setMainCamera(camera);
	}
	
	gust::Engine::get().simulate();
	gust::Engine::get().shutdown();

	std::cin.get();
	return 0;
}