#pragma once

/**
 * @file Allocators.hpp
 * @brief Allocators header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vector>
#include <algorithm>
#include <cmath>
#include "Debugging.hpp"

namespace gust
{
	/**
	 * @class StackAllocator
	 * @brief Allocates data in a stack.
	 */
	class StackAllocator
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		StackAllocator();

		/**
		 * @brief Constructor.
		 * @param Stack size.
		 */
		StackAllocator(size_t size);

		/**
		 * @brief Copy constructor.
		 * @param Source.
		 */
		StackAllocator(const StackAllocator& other);
		
		/**
		 * @brief Destructor.
		 */
		~StackAllocator();

		/**
		 * @brief Assignment operator.
		 * @param Source.
		 * @return This.
		 */
		StackAllocator& operator=(const StackAllocator& other);

		/**
		 * @brief Allocate N bytes of data in the stack.
		 * @param Number of bytes to allocate.
		 * @return Pointer to the allocated memory.
		 */
		void* allocate(size_t n);

		/**
		 * @brief Allocate N bytes of data in the stack.
		 * @param Number of bytes to allocate.
		 * @param Alignment requirement.
		 * @return Pointer to the allocated memory.
		 * @note The alignment must be a power of 2.
		 */
		void* allocate(size_t n, size_t a);

		/**
		 * @brief Free all memory in the stack.
		 */
		void free();

		/**
		 * @brief Initialize a new stack.
		 * @param Number of bytes to allocate.
		 * @note This will free the current stack.
		 */
		void initialize(size_t n);

		/**
		 * @brief Get stack size.
		 * @return Stack size.
		 */
		inline size_t getSize() const
		{
			return m_size;
		}

		/**
		 * @brief Get number of bytes allocated.
		 * @return Number of bytes allocated.
		 */
		inline size_t getAllocated() const
		{
			return static_cast<size_t>(m_top - m_data);
		}

	private:

		/** Pointer to the base of the stack. */
		unsigned char* m_data;

		/** Pointer to the top of the stack. */
		unsigned char* m_top;

		/** Number of bytes allocated in the stack. */
		size_t m_size;
	};

	/**
	 * @brief PoolAllocator
	 * @brief Allocates fixed chunks of data in a stack like fashion.
	 */
	class PoolAllocator
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		PoolAllocator();

		/**
		 * @brief Constructor.
		 * @param Size of chunks.
		 * @param Number of chunks to store.
		 * @param Chunk alignment.
		 * @note Chunk alignment must be a power of 2.
		 */
		PoolAllocator(size_t size, size_t count, size_t alignment);

		/**
		 * @brief Copy constructor.
		 * @param Other.
		 */
		PoolAllocator(const PoolAllocator& other);

		/**
		 * @brief Destructor.
		 */
		~PoolAllocator();

		/**
		 * @brief Assignment operator.
		 * @param Other.
		 * @return This.
		 */
		PoolAllocator& operator=(const PoolAllocator& other);

		/**
		 * @brief Get the total number of bytes allocated on the pool.
		 * @return Total number of bytes allocated on the pool.
		 */
		inline size_t getSize() const
		{
			return m_alignment + (m_chunkCount * m_chunkSize);
		}

		/**
		 * @brief Get number of bytes allocated.
		 * @return Number of bytes allocated.
		 */
		inline size_t getAllocated() const
		{
			return static_cast<size_t>(m_top - m_data);
		}

		/**
		 * @brief Get alignment for data in this pool.
		 * @return Alignment for data in this pool.
		 */
		inline size_t getAlignment() const
		{
			return m_alignment;
		}

		/**
		 * @brief Get max number of chunks you can allocate.
		 * @return Max number of chunks you can allocate.
		 */
		inline size_t getMaxChunkCount() const
		{
			return m_chunkCount;
		}

		/**
		 * @brief Get size of chunks.
		 * @return Chunk size.
		 */
		inline size_t getChunkSize() const
		{
			return m_chunkSize;
		}

		/**
		 * @brief Get number of chunks allocated.
		 * @return Number of chunks allocated.
		 */
		inline size_t getChunkCount() const
		{
			return (m_top - m_data) / m_chunkSize;
		}

		/**
		 * @brief Allocate a chunk.
		 * @return Pointer to the allocated chunk.
		 */
		void* allocate();

		/**
		 * @brief Free all memory in the pool.
		 */
		void free();

		/**
		 * @brief Initialize a new pool.
		 * @param Size of chunks.
		 * @param Number of chunks to store.
		 * @param Chunk alignment.
		 * @note Chunk alignment must be a power of 2.
		 */
		void initialize(size_t size, size_t count, size_t alignment);

	private:

		/** Pointer to the base of the pool. */
		unsigned char* m_data;

		/** Pointer to the top of the pool. */
		unsigned char* m_top;

		/** Alignment for data on the pool. */
		size_t m_alignment;

		/** Number of chunks stored on the pool. */
		size_t m_chunkCount;

		/** Chunk size. */
		size_t m_chunkSize;
	};

	/**
	 * @class ResourceAllocatorBase
	 * @brief Base class for resource allocators.
	 */
	class ResourceAllocatorBase
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		ResourceAllocatorBase();

		/**
		 * @brief Default destructor.
		 */
		virtual ~ResourceAllocatorBase() = default;

		/**
		 * @brief Get max number of resources.
		 * @return Max number of resources.
		 */
		inline size_t getMaxResourceCount() const
		{
			return m_maxResourceCount;
		}

		/**
		 * @brief Check if a handle to a resource is allocated.
		 * @param Handle to resource.
		 * @return If it is allocated or not.
		 */
		inline bool isAllocated(size_t handle) const
		{
			gAssert(handle < m_maxResourceCount);
			return m_allocation[handle];
		}

		/**
		 * @brief Get number of resources in use.
		 * @return Number of resources in use.
		 */
		inline size_t getResourceCount() const
		{
			size_t counter = 0;

			for (size_t i = 0; i < m_maxResourceCount; ++i)
				if (isAllocated(i))
					++counter;

			return counter;
		}

	protected:

		/** Max number of resources. */
		size_t m_maxResourceCount;

		/** Allocation table. */
		std::vector<unsigned char> m_allocation = {};
	};

	/**
	 * @class ResourceAllocator
	 * @brief A pool allocator that allows for deleting data mid stack.
	 * and resizing the stack.
	 * @see Handle
	 */
	template<class T>
	class ResourceAllocator : public ResourceAllocatorBase
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		ResourceAllocator() : ResourceAllocatorBase()
		{

		}

		/**
		 * @brief Constructor.
		 * @param Number of resources stored.
		 */
		ResourceAllocator(size_t count) : ResourceAllocatorBase()
		{
			m_maxResourceCount = count;
			
			// Resize resources and allocation table
			m_allocation.resize(m_maxResourceCount, false);
			m_resources.resize(m_maxResourceCount);
		}

		/**
		 * @brief Destructor.
		 */
		~ResourceAllocator() = default;

		/**
		 * @brief Get pointer to resource from it's handle.
		 * @param Resource handle.
		 * @return Resource.
		 */
		inline T* getResourceByHandle(size_t handle)
		{
			gAssert(handle < m_maxResourceCount);
			return &m_resources.at(handle);
		}

		/**
		 * @brief Allocate a new resource.
		 * @return Resource handle.
		 * @note The constructor for the resource will not be called.
		 */
		size_t allocate()
		{
			// Make sure we have enough space
			gAssert(getResourceCount() != getMaxResourceCount());

			// Search for an appropriate index
			size_t index;
			for(index = 0; index < m_maxResourceCount; ++index)
				if (!isAllocated(index))
					break;

			m_allocation[index] = true;
			return index;
		}

		/**
		 * @brief Deallocate a resource.
		 * @param Resource handle.
		 * @note This will call the destructor.
		 */
		void deallocate(size_t handle)
		{
			gAssert(handle < m_maxResourceCount);

			if (!isAllocated(handle))
				return;

			m_resources[handle].~T();
			m_allocation[handle] = false;
		}

		/**
		 * @brief Resize the resource array to fit more resources.
		 * @param New size.
		 * @bool Should we maintain the current resources?
		 * @note If the new size is less than the old size,
		 * the old resources will not be maintained.
		 */
		void resize(size_t newSize, bool maintain)
		{
			if (newSize == m_maxResourceCount)
				return;

			// Resize and maintain old data
			if (maintain && newSize > m_maxResourceCount)
			{
				m_maxResourceCount = newSize;

				m_allocation.resize(m_maxResourceCount);
				m_resources.resize(m_maxResourceCount);
			}
			// Ignore old data
			else
			{
				m_allocation.clear();
				m_resources.clear();

				m_maxResourceCount = newSize;

				m_allocation.resize(m_maxResourceCount);
				m_resources.resize(m_maxResourceCount);
			}
		}

	private:

		/** Resources. */
		std::vector<T> m_resources = {};
	};

	/**
	 * @class Handle
	 * @brief A handle to a resource in a resource allocator.
	 * @see ResourceAllocator
	 */
	template<class T>
	class Handle
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Handle() : m_resourceAllocator(nullptr), m_handle(0)
		{

		}

		/**
		 * @brief Constructor.
		 * @tparam Type of other handle.
		 */
		template<class U>
		Handle(const Handle<U>& other) :
			m_resourceAllocator(other.getResourceAllocator()),
			m_handle(other.getHandle())
		{

		}

		/**
		 * @brief Constructor.
		 * @param Resource allocator.
		 * @param Handle.
		 */
		Handle(ResourceAllocatorBase* allocator, size_t handle) :
			m_resourceAllocator(allocator),
			m_handle(handle)
		{

		}

		/**
		 * @brief Destructor.
		 */
		~Handle() = default;

		/**
		 * @brief Get a null handle.
		 * @return A null handle.
		 */
		inline static Handle<T> nullHandle()
		{
			return Handle<T>(nullptr, 0);
		}

		/**
		 * @brief Get handle.
		 * @return Handle.
		 */
		inline size_t getHandle() const
		{
			return m_handle;
		}

		/**
		 * @brief Get resource allocator.
		 * @return Resource allocator.
		 */
		inline ResourceAllocatorBase* getResourceAllocator() const
		{
			return m_resourceAllocator;
		}

		/**
		 * @brief Access operator.
		 * @return Resource.
		 */
		inline T* operator->() const
		{
			return get();
		}

		/**
		 * @brief Get pointer to resource.
		 * @return Pointer to resource.
		 */
		inline T* get() const
		{
			return m_resourceAllocator == nullptr ? nullptr : static_cast<T*>(static_cast<ResourceAllocator<T>*>(m_resourceAllocator)->getResourceByHandle(m_handle));
		}

		/**
		 * @brief Equvilence check.
		 * @param Other.
		 * @return If the handles are equal.
		 */
		friend bool operator==(const Handle<T>& lh, const Handle<T>& rh)
		{
			return (lh.m_resourceAllocator == rh.m_resourceAllocator) && (lh.m_handle == rh.m_handle);
		}

		/**
		 * @brief Unequvilence check.
		 * @param Other.
		 * @return If the handles are not equal.
		 */
		friend bool operator!=(const Handle<T>& lh, const Handle<T>& rh)
		{
			return (lh.m_resourceAllocator != rh.m_resourceAllocator) || (lh.m_handle != rh.m_handle);
		}

		/**
		 * @brief Assignment operator.
		 * @tparam Type of other handle.
		 * @param Handle to set this handle equal to.
		 * @return Reference to this handle.
		 * @note U should derive from T.
		 */
		template<class U>
		inline Handle<T>& operator=(const Handle<U>& other)
		{
			static_assert(std::is_same<T, U>::value || std::is_base_of<T, U>::value, "U must derive from T or U must equal T.");
			m_resourceAllocator = other.m_resourceAllocator;
			m_handle = other.m_handle;
			return *this;
		}

	private:

		/** Resource allocator. */
		ResourceAllocatorBase* m_resourceAllocator;

		/** Resources handle. */
		size_t m_handle;
	};
}
