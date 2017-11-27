#include <iostream>
#include <GUST-Core\Allocators.hpp>

struct TestStruct
{
	float m1 = 0;
	bool m2 = false;
	float m3 = 0;
};

int main()
{
	auto alloc = gust::core::ResourceAllocator<TestStruct>(15, 4);

	gust::core::Handle<TestStruct> h1;

	for (size_t i = 0; i < 5; i++)
		if (i == 4)
		{
			h1 = gust::core::Handle<TestStruct>(&alloc, alloc.allocate());
			::new(h1.get())(TestStruct);
		}

	h1->m3 = 10.1f;

	std::cout << h1->m1 << ' ' << h1->m2 << ' ' << h1->m3 << '\n';

	std::cin.get();
	return 0;
}