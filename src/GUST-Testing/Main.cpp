#include <iostream>
#include <GUST-Engine\Engine.hpp>

struct TestStruct
{
	float m1 = 0;
	float m3 = 0;
	bool m2 = false;
};

int main()
{
	gust::Engine::get().startup("Test Game", 1280, 720);
	gust::Engine::get().renderer.createCamera();
	gust::Engine::get().simulate();
	gust::Engine::get().shutdown();

	std::cin.get();
	return 0;
}