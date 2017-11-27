#pragma once

/**
 * @file Allocators.hpp
 * @brief Allocators header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vector>
#include <algorithm>
#include <assert.h>

namespace gust
{
	namespace core
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
		 * @class ResourceAllocator
		 * @brief A pool allocator that allows for deleting data mid stack.
		 * and resizing the stack.
		 * @see Handle
		 */
		template<class T>
		class ResourceAllocator
		{
		public:

			/**
			 * @brief Default constructor.
			 */
			ResourceAllocator() : 
				m_data(nullptr), 
				m_top(nullptr),
				m_alignment(0),
				m_maxResourceCount(0),
				m_offset(0)
			{

			}

			/**
			 * @brief Constructor.
			 * @param Number of resources stored.
			 * @param Resource alignment.
			 * @note Chunk alignment must be a power of 2.
			 */
			ResourceAllocator(size_t count, size_t alignment) : 
				m_alignment(alignment),
				m_maxResourceCount(count)
			{
				m_data = new unsigned char[(sizeof(T) * count) + m_alignment + count];
				m_offset = (m_alignment - 1) & reinterpret_cast<size_t>(m_data + count);

				// Reset allocation table
				for (size_t i = 0; i < m_maxResourceCount; i++)
					m_data[i] = 0;
			}

			/**
			 * @brief Destructor.
			 */
			~ResourceAllocator()
			{
				delete m_data;
			}

			/**
			 * @brief Get size of resources.
			 * @return Size of resources.
			 */
			inline constexpr size_t getResourceSize() const
			{
				return sizeof(T);
			}

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
				return m_data[handle] != 0;
			}

			/**
			 * @brief Get number of resources in use.
			 * @return Number of resources in use.
			 */
			inline size_t getResourceCount() const
			{
				size_t counter = 0;

				for (size_t i = 0; i < m_maxResourceCount; i++)
					if (isAllocated(i))
						++counter;

				return counter;
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

				return reinterpret_cast<T*>((m_data + m_offset + m_maxResourceCount) + (handle * sizeof(T)));
			}

			/**
			 * @brief Allocate a new resource.
			 * @note The constructor for the resource will no 
			 * have been called, so make sure you do that.
			 * @note Resource handle.
			 */
			size_t allocate() const
			{
				// Make sure we have enough space
				assert(getResourceCount() != getMaxResourceCount());

				// Search for an appropriate index
				size_t index = 0;
				for(size_t i = 0; i < m_maxResourceCount; i++)
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

		private:

			/** Pointer to base of the stack. */
			unsigned char* m_data;

			/** Max number of resources. */
			size_t m_maxResourceCount;

			/** Data offset. */
			size_t m_offset;

			/** Resource alignment. */
			size_t m_alignment;
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
			 * @param Resource allocator.
			 * @param Handle.
			 */
			Handle(ResourceAllocator<T>* allocator, size_t handle) : 
				m_resourceAllocator(allocator),
				m_handle(handle)
			{

			}

			/**
			 * @brief Destructor.
			 */
			~Handle() = default;

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
			inline ResourceAllocator<T>* getResourceAllocator() const
			{
				return m_resourceAllocator;
			}

			/**
			 * @brief Access operator.
			 * @return Resource.
			 */
			T* operator->() const 
			{
				return m_resourceAllocator->getResourceByHandle(m_handle);
			}

			/**
			 * @brief Get pointer to resource.
			 * @return Pointer to resource.
			 */
			T* get() const
			{
				return m_resourceAllocator->getResourceByHandle(m_handle);
			}
			
		private:

			/** Resource allocator. */
			ResourceAllocator<T>* m_resourceAllocator;

			/** Resources handle. */
			size_t m_handle;
		};
	}
}