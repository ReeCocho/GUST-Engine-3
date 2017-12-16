#include <iostream>
#include <GUST-Engine\Engine.hpp>
#include <GUST-Engine\Components\Transform.hpp>

class TestComponent : public gust::Component<TestComponent>
{
public:

	TestComponent(gust::Entity entity, gust::Handle<TestComponent> handle) : gust::Component<TestComponent>(entity, handle)
	{

	}

	std::string m_words = "Hello world!";
};

class TestComponentSystem : public gust::System
{
public:

	TestComponentSystem(gust::Scene* scene) : gust::System(scene)
	{
		initialize<TestComponent>();
	}

	void onTick(float deltaTime) override
	{
		auto component = getComponent<TestComponent>();
		std::cout << component->m_words << '\n';
	}
};

int main()
{
	gust::Engine::get().startup("Test Game", 1280, 720);

	gust::Engine::get().scene.addSystem<gust::TransformSystem>();
	gust::Engine::get().scene.addSystem<TestComponentSystem>();

	auto entity = gust::Entity(&gust::Engine::get().scene);
	entity.addComponent<TestComponent>();

	auto camera = gust::Engine::get().renderer.createCamera();
	gust::Engine::get().renderer.setMainCamera(camera);

	gust::Engine::get().simulate();
	gust::Engine::get().shutdown();

	std::cin.get();
	return 0;
}