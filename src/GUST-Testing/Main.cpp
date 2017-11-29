#include <iostream>
#include <GUST-Engine\Utilities\Allocators.hpp>

struct TestStruct
{
	float m1 = 0;
	float m3 = 0;
	bool m2 = false;
};

int main()
{
	auto alloc = gust::ResourceAllocator<TestStruct>(150, 4);

	gust::Handle<TestStruct> h1;

	for (size_t i = 0; i < 5; i++)
		if (i == 3)
		{
			h1 = gust::Handle<TestStruct>(&alloc, alloc.allocate());
			::new(h1.get())(TestStruct);
		}

	h1->m3 = 10.1f;

	std::cout << h1->m1 << ' ' << h1->m2 << ' ' << h1->m3 << '\n';

	std::cin.get();
	return 0;
}