/**
 * @file Mesh.hpp
 * @brief Mesh header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Vulkan.hpp"
#include "Math.hpp"
#include <vector>

namespace gust
{
	/**
	 * @struct Vertex
	 * @brief Vertex info.
	 * @see Mesh
	 */
	struct Vertex
	{
		/** Position. */
		glm::vec3 position = {};

		/** UV. */
		glm::vec2 uv = {};

		/** Normal. */
		glm::vec3 normal = {};

		/** Tangent. */
		glm::vec3 tangent = {};

		/**
		 * @brief Describes how to pass data to vertex shader.
		 * @return Vertex input bidning description.
		 */
		static vk::VertexInputBindingDescription getBindingDescription();

		/**
		 * @brief Describes each input to the vertex shader.
		 * @return Array descriving each input of the vertex shader.
		 */
		static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions();

		/**
		 * @brief Comparison operator.
		 * @param Vertex to compare with.
		 * @return If they are equal.
		 */
		bool operator==(const Vertex& other) const
		{
			return position == other.position && normal == other.normal && uv == other.uv && tangent == other.tangent;
		}
	};



	/**
	 * @class Mesh
	 * @brief Stores data about a renderable mesh.
	 */
	class Mesh
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		Mesh() = default;

		/**
		 * @brief Destructor.
		 */
		~Mesh();

		/**
		* @brief Constructor.
		* @param Graphics context.
		* @param Meshes indices.
		* @param Meshes vertecies.
		*/
		Mesh
		(
			Graphics* graphics,
			const std::vector<uint32_t>& indices,
			const std::vector<glm::vec3>& vertices
		);

		/**
		* @brief Constructor.
		* @param Graphics context.
		* @param Meshes indices.
		* @param Meshes vertecies.
		* @param Meshes UVs.
		*/
		Mesh
		(
			Graphics* graphics,
			const std::vector<uint32_t>& indices,
			const std::vector<glm::vec3>& vertices,
			const std::vector<glm::vec2>& uvs
		);

		/**
		* @brief Constructor.
		* @param Graphics context.
		* @param Meshes indices.
		* @param Meshes vertecies.
		* @param Meshes UVs.
		* @param Meshes normals.
		*/
		Mesh
		(
			Graphics* graphics,
			const std::vector<uint32_t>& indices,
			const std::vector<glm::vec3>& vertices,
			const std::vector<glm::vec2>& uvs,
			const std::vector<glm::vec3>& normals
		);

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Meshes indices.
		 * @param Meshes vertecies.
		 * @param Meshes UVs.
		 * @param Meshes normals.
		 * @param Meshes tangents.
		 * @param Meshes bitangents.
		 */
		Mesh
		(
			Graphics* graphics,
			const std::vector<uint32_t>& indices,
			const std::vector<glm::vec3>& vertices,
			const std::vector<glm::vec2>& uvs,
			const std::vector<glm::vec3>& normals,
			const std::vector<glm::vec3>& tangents
		);

		/**
		 * @brief Get number of indices.
		 * @return Number of indices.
		 */
		inline uint32_t getIndexCount() const
		{
			return static_cast<uint32_t>(m_indices.size());
		}

		/**
		 * @brief Get vertex uniform buffer
		 * @return Vertex uniform buffer.
		 */
		inline const Buffer& getVertexUniformBuffer() const
		{
			return m_vertexBuffer;
		}

		/**
		 * @brief Get index uniform buffer
		 * @return Index uniform buffer.
		 */
		inline const Buffer& getIndexUniformBuffer() const
		{
			return m_indexBuffer;
		}

		/**
		 * @brief Calculates tangents for the mesh.
		 */
		void calculateTangents();

	private:

		/** Vertex data. */
		std::vector<Vertex> m_vertices;

		/** Index data. */
		std::vector<uint32_t> m_indices;

		/** Vertex uniform buffer. */
		Buffer m_vertexBuffer = {};

		/** Index uniform buffer. */
		Buffer m_indexBuffer = {};
	};
}