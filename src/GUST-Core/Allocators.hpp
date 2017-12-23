#pragma once

/**
 * @file Allocators.hpp
 * @brief Allocators header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Debugging.hpp"
#include <vector>
#include <algorithm>
#include <assert.h>

namespace gust
{
	/**
	 * @class StackAllocator
	 * @brief Allocates data in a stack like fashion.
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
		 * @brief Destructor.
		 */
		~StackAllocator();

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
		void freeAll();

		/**
		 * @brief Initialize a new stack.
		 * @param Number of bytes to allocate.
		 * @note This will not free the current stack.
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
			return m_top - m_data;
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
		 * @brief Destructor.
		 */
		~PoolAllocator();

		/**
		 * @brief Get the total number of bytes allocated on the stack.
		 * @return Total number of bytes allocated on the stack.
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
			return m_top - m_data;
		}

		/**
		 * @brief Get alignment for data in this stack.
		 * @return Alignment for data in this stack.
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
		 * @brief Free all memory in the stack.
		 */
		void freeAll();

		/**
		 * @brief Initialize a new stack.
		 * @param Size of chunks.
		 * @param Number of chunks to store.
		 * @param Chunk alignment.
		 * @note This will not free the current stack.
		 * @note Chunk alignment must be a power of 2.
		 */
		void initialize(size_t size, size_t count, size_t alignment);

	private:

		/** Pointer to the base of the stack. */
		unsigned char* m_data;

		/** Pointer to the top of the stack. */
		unsigned char* m_top;

		/** Alignment for data on the stack. */
		size_t m_alignment;

		/** Number of chunks stored on the stack. */
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
			assert(handle < m_maxResourceCount);
			return m_data[handle] != 0;
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

		/**
		 * @brief Get size of resources.
		 * @return Size of resources.
		 */
		inline size_t getResourceSize() const
		{
			return m_clampedDataSize;
		}

		/**
		 * @brief Get a void* to a resource.
		 * @param Resource handle.
		 * @return Void* to the resource.
		 */
		inline void* getRawResourceByHandle(size_t handle)
		{
			// Make sure the handle is in use
			if (!isAllocated(handle))
				return nullptr;

			return reinterpret_cast<void*>((m_data + m_offset + m_maxResourceCount) + (handle * m_clampedDataSize));
		}

	protected:

		/** Pointer to base of the stack. */
		unsigned char* m_data;

		/** Max number of resources. */
		size_t m_maxResourceCount;

		/** Data offset. */
		size_t m_offset;

		/** Size of data clamped to be a multiple of the alignment. */
		size_t m_clampedDataSize;

		/** Resource alignment. */
		size_t m_alignment;
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
		 * @param Resource alignment.
		 * @note Chunk alignment must be a power of 2.
		 */
		ResourceAllocator(size_t count, size_t alignment) : ResourceAllocatorBase()
		{
			m_alignment = alignment;
			m_maxResourceCount = count;

			m_clampedDataSize = static_cast<size_t>(ceil(static_cast<float>(sizeof(T)) / static_cast<float>(m_alignment))) * m_alignment;
			m_data = new unsigned char[(m_clampedDataSize * count) + m_alignment + count];
			m_offset = (m_alignment - 1) & reinterpret_cast<size_t>(m_data + count);

			// Reset allocation table
			for (size_t i = 0; i < m_maxResourceCount; ++i)
				m_data[i] = 0;
		}

		/**
		 * @brief Destructor.
		 */
		~ResourceAllocator()
		{
			if (m_data != nullptr)
			{
				// Call destructors for all allocated data
				for (size_t i = 0; i < m_maxResourceCount; ++i)
				{
					if (isAllocated(i))
					{
						T* data = reinterpret_cast<T*>(m_data + m_maxResourceCount + m_offset + (i * m_clampedDataSize));
						data->~T();
					}
				}

				delete[] m_data;
			}
		}

		/**
		 * @brief Get pointer to resource from it's handle.
		 * @param Resource handle.
		 * @return Resource.
		 */
		inline T* getResourceByHandle(size_t handle) const
		{
			// Make sure the handle is in use
			if (!isAllocated(handle))
				return nullptr;

			return reinterpret_cast<T*>((m_data + m_offset + m_maxResourceCount) + (handle * m_clampedDataSize));
		}

		/**
		 * @brief Allocate a new resource.
		 * @return Resource handle.
		 * @note The constructor for the resource will not
		 * have been called, so make sure you do that.
		 */
		size_t allocate() const
		{
			// Make sure we have enough space
			assert(getResourceCount() != getMaxResourceCount());

			// Search for an appropriate index
			size_t index = 0;
			for(size_t i = 0; i < m_maxResourceCount; ++i)
				if (!isAllocated(i))
				{
					index = i;
					break;
				}

			m_data[index] = 1;
			return index;
		}

		/**
		 * @brief Deallocate a resource.
		 * @param Resource handle.
		 * @note This will call the destructor so you don't have to.
		 */
		void deallocate(size_t handle) const
		{
			if (!isAllocated(handle))
				return;

			T* resource = getResourceByHandle(handle);
			resource->~T();
			m_data[handle] = 0;
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
			// Resize and maintain old data
			if (maintain && newSize >= m_maxResourceCount)
			{
				unsigned char* oldData = m_data;
				size_t oldSize = m_maxResourceCount;
				size_t oldOffset = m_offset;

				// Create new data
				m_maxResourceCount = newSize;
				m_data = new unsigned char[(m_clampedDataSize * newSize) + m_alignment + newSize];
				m_offset = (m_alignment - 1) & reinterpret_cast<size_t>(m_data + newSize);

				// Reset allocation table
				for (size_t i = 0; i < m_maxResourceCount; ++i)
					m_data[i] = (i < oldSize ? oldData[i] : 0);

				// Set old data
				for (size_t i = 0; i < oldSize * m_clampedDataSize; ++i)
				{
					size_t newIndex = (m_maxResourceCount + m_offset) + i;
					size_t oldIndex = (oldSize + oldOffset) + i;
					m_data[newIndex] = oldData[oldIndex];
				}

				// Delete old data.
				delete[] oldData;
			}
			// Ignore old data
			else
			{
				delete[] m_data;

				// Create new data
				m_maxResourceCount = newSize;
				m_data = new unsigned char[(m_clampedDataSize * newSize) + m_alignment + newSize];
				m_offset = (m_alignment - 1) & reinterpret_cast<size_t>(m_data + newSize);

				// Reset allocation table
				for (size_t i = 0; i < m_maxResourceCount; ++i)
					m_data[i] = 0;
			}
		}
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
			// static_assert(std::is_base_of<T, U>::value, "U must derive from T");
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
			return m_resourceAllocator == nullptr ? nullptr : static_cast<T*>(m_resourceAllocator->getRawResourceByHandle(m_handle));
		}

		/**
		 * @brief Get pointer to resource.
		 * @return Pointer to resource.
		 */
		inline T* get() const
		{
			return static_cast<T*>(m_resourceAllocator->getRawResourceByHandle(m_handle));
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
			static_assert(std::is_base_of<T, U>::value, "U must derive from T");
			m_resourceAllocator = other.getResourceAllocator();
			m_handle = other.getHandle();
			return *this;
		}

		/**
		 * @brief Assignment operator.
		 * @tparam Type of other handle.
		 * @param Handle to set this handle equal to.
		 * @return Reference to this handle.
		 */
		template<>
		inline Handle<T>& operator=<T>(const Handle<T>& other)
		{
			m_resourceAllocator = other.getResourceAllocator();
			m_handle = other.getHandle();
			return *this;
		}

	private:

		/** Resource allocator. */
		ResourceAllocatorBase* m_resourceAllocator;

		/** Resources handle. */
		size_t m_handle;
	};
}