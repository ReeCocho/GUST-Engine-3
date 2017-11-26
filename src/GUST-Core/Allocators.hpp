#pragma once

/**
 * @file Allocators.hpp
 * @brief Allocators header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <vector>
#include <algorithm>

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
	}
}