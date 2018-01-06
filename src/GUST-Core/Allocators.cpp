#include "Allocators.hpp"

namespace gust
{
	StackAllocator::StackAllocator() : m_data(nullptr), m_top(nullptr), m_size(0)
	{

	}

	StackAllocator::StackAllocator(size_t size) :  m_size(size)
	{
		m_data = new unsigned char[size];
		gAssert(m_data);
		m_top = m_data;
	}

	StackAllocator::~StackAllocator()
	{
		gAssert(m_data);  
		delete m_data;
	}

	void* StackAllocator::allocate(size_t n)
	{
		// Make sure we have enough data to allocate
		gAssert(static_cast<size_t>((m_top + n) - m_data) <= m_size);

		void* data = reinterpret_cast<void*>(m_top);
		m_top += n;
		return data;
	}

	void* StackAllocator::allocate(size_t n, size_t a)
	{
		size_t offset = (a - 1) & reinterpret_cast<size_t>(m_top);

		// Make sure we have enough data to allocate
		gAssert(static_cast<size_t>((m_top + offset + n) - m_data) <= m_size);

		void* data = reinterpret_cast<void*>(m_top + offset);
		m_top += n + offset;
		return data;
	}

	void StackAllocator::freeAll()
	{
		gAssert(m_data);  
		delete m_data;
		m_size = 0;
	}

	void StackAllocator::initialize(size_t n)
	{
		m_data = new unsigned char[n];
		gAssert(m_data);
		m_size = n;
	}



	PoolAllocator::PoolAllocator() : 
		m_data(nullptr), 
		m_top(nullptr), 
		m_alignment(0), 
		m_chunkCount(0), 
		m_chunkSize(0)
	{

	}

	PoolAllocator::PoolAllocator(size_t size, size_t count, size_t alignment) : 
		m_alignment(alignment), 
		m_chunkSize(size), 
		m_chunkCount(count)
	{
		m_data = new unsigned char[getSize()];
		gAssert(m_data);
		size_t offset = (alignment - 1) & reinterpret_cast<size_t>(m_data);
		m_top = m_data + offset;
	}

	PoolAllocator::~PoolAllocator()
	{
		gAssert(m_data); 
		delete m_data;
	}

	void* PoolAllocator::allocate()
	{
		// Make sure we have enough room
		gAssert(getChunkCount() != getMaxChunkCount());

		void* data = reinterpret_cast<void*>(m_top);
		m_top += m_chunkSize;
		return data;
	}

	void PoolAllocator::freeAll()
	{
		gAssert(m_data);
		delete m_data;
		m_alignment = 0;
		m_chunkCount = 0;
		m_chunkSize = 0;
	}

	void PoolAllocator::initialize(size_t size, size_t count, size_t alignment)
	{
		m_alignment = alignment;
		m_chunkCount = count;
		m_chunkSize = size;

		m_data = new unsigned char[getSize()];
		gAssert(m_data);
		size_t offset = (alignment - 1) & reinterpret_cast<size_t>(m_data);
		m_top = m_data + offset;
	}



	ResourceAllocatorBase::ResourceAllocatorBase() : m_maxResourceCount(0)
	{

	}
}