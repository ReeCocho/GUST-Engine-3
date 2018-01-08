#include "Allocators.hpp"

namespace gust
{
	StackAllocator::StackAllocator() : m_data(nullptr), m_top(nullptr), m_size(0)
	{

	}

	StackAllocator::StackAllocator(size_t size) :  m_size(size)
	{
		m_data = new unsigned char[m_size];
		gAssert(m_data);
		m_top = m_data;
	}

	StackAllocator::StackAllocator(const StackAllocator& other) : m_size(other.m_size)
	{
		m_data = new unsigned char[m_size];
		gAssert(m_data);

		// Copy data from original stack
		std::memcpy(m_data, other.m_data, m_size);

		// Set top
		m_top = m_data + other.getAllocated();
	}

	StackAllocator::~StackAllocator()
	{
		if(m_data) delete[] m_data;
	}

	StackAllocator& StackAllocator::operator=(const StackAllocator& other)
	{
		// Free old data if needed
		free();

		m_size = other.m_size;
		m_data = new unsigned char[m_size];
		gAssert(m_data);

		// Copy other data
		std::memcpy(m_data, other.m_data, m_size);

		m_top = m_data + other.getAllocated();

		return *this;
	}

	void* StackAllocator::allocate(size_t n)
	{
		// Make sure the data exists
		gAssert(m_data);

		// Make sure we have enough space to allocate
		gAssert(static_cast<size_t>((m_top + n) - m_data) <= m_size);

		void* data = reinterpret_cast<void*>(m_top);
		m_top += n;
		return data;
	}

	void* StackAllocator::allocate(size_t n, size_t a)
	{
		// Make sure the data exists
		gAssert(m_data);

		// Make sure a is a power of 2
		gAssert((a & (a - 1)) == 0);

		// Calculate the offset from the top
		size_t offset = (a - 1) & reinterpret_cast<size_t>(m_top);

		// Make sure we have enough space to allocate
		gAssert(static_cast<size_t>((m_top + offset + n) - m_data) <= m_size);

		void* data = reinterpret_cast<void*>(m_top + offset);
		m_top += n + offset;
		return data;
	}

	void StackAllocator::free()
	{
		gAssert(m_data);  
		delete[] m_data;
		m_data = nullptr;
		m_top = nullptr;
		m_size = 0;
	}

	void StackAllocator::initialize(size_t n)
	{
		// Free original data (if any)
		free();

		m_data = new unsigned char[n];
		gAssert(m_data);
		
		m_size = n;
		m_top = m_data;
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
		// Make sure the alignment is a power of 2
		gAssert((alignment & (alignment - 1)) == 0);

		m_data = new unsigned char[getSize()];
		gAssert(m_data);

		// Calculate an offset.
		size_t offset = (alignment - 1) & reinterpret_cast<size_t>(m_data);
		m_top = m_data + offset;
	}

	PoolAllocator::PoolAllocator(const PoolAllocator& other)
	{
		m_chunkSize = other.m_chunkSize;
		m_alignment = other.m_alignment;
		m_chunkCount = other.m_chunkCount;
	
		m_data = new unsigned char[getSize()];
		gAssert(m_data);

		// Calculate an offset.
		size_t offset = (m_alignment - 1) & reinterpret_cast<size_t>(m_data);
		m_top = m_data + offset;

		// Copy other data
		std::memcpy(m_top, other.m_data + ((m_alignment - 1) & reinterpret_cast<size_t>(other.m_data)), other.getSize());
	}

	PoolAllocator::~PoolAllocator()
	{
		if(m_data) delete[] m_data;
	}

	PoolAllocator& PoolAllocator::operator=(const PoolAllocator& other)
	{
		// Free old data if needed
		free();

		m_chunkSize = other.m_chunkSize;
		m_alignment = other.m_alignment;
		m_chunkCount = other.m_chunkCount;

		m_data = new unsigned char[getSize()];
		gAssert(m_data);

		// Calculate an offset.
		size_t offset = (m_alignment - 1) & reinterpret_cast<size_t>(m_data);
		m_top = m_data + offset;

		// Copy other data
		std::memcpy(m_top, other.m_data + ((m_alignment - 1) & reinterpret_cast<size_t>(other.m_data)), other.getSize());

		return *this;
	}

	void* PoolAllocator::allocate()
	{
		// Make sure we have enough room
		gAssert(getChunkCount() != getMaxChunkCount());

		void* data = reinterpret_cast<void*>(m_top);
		m_top += m_chunkSize;
		return data;
	}

	void PoolAllocator::free()
	{
		if(m_data) delete[] m_data;
		m_alignment = 0;
		m_chunkCount = 0;
		m_chunkSize = 0;
	}

	void PoolAllocator::initialize(size_t size, size_t count, size_t alignment)
	{
		// Free old data if needed
		free();

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