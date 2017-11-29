/**
 * @file ResourceManager.hpp
 * @brief Resource manager header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "../Utilities/Allocators.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Material.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"

namespace gust
{
	/**
	 * @class ResourceManager
	 * @brief Manages resources.
	 */
	class ResourceManager
	{
	public:

		/**
		 * @brief Default constructor.
		 */
		ResourceManager() = default;

		/**
		 * @brief Constructor.
		 * @param Default mesh count.
		 * @param Default material count.
		 * @param Default shader count.
		 * @param Default texture count.
		 */
		ResourceManager
		(
			size_t meshCount, 
			size_t materialCount, 
			size_t shaderCount, 
			size_t textureCount
		);

		/**
		 * @brief Destructor.
		 */
		~ResourceManager();

	private:

		
	};
}