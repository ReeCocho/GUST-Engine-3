#pragma once

/** 
 * @file Texture.hpp
 * @brief Texture header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Graphics.hpp"

namespace gust
{
	/**
	 * @class Texture
	 * @brief Holds texture information.
	 */
	class Texture
	{
	public:
		
		/**
		 * @brief Default constructor.
		 */
		Texture() = default;

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Image.
		 * @param Image view.
		 * @param Sampler.
		 * @param Image memory.
		 * @param Width.
		 * @param Height.
		 */
		Texture
		(
			Graphics* graphics, 
			vk::Image image, 
			vk::ImageView& imageView, 
			vk::Sampler sampler, 
			vk::DeviceMemory memory, 
			uint32_t width, 
			uint32_t height
		);

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Path to file containing image.
		 * @param Texture filtering.
		 */
		Texture(Graphics* graphics, const std::string& path, vk::Filter filter);

		/**
		 * @brief Destructor.
		 */
		virtual ~Texture();

		/**
		 * @brief Get texture width.
		 * @return Texture width.
		 */
		inline uint32_t getWidth() const
		{
			return m_width;
		}

		/**
		 * @brief Get texture height.
		 * @return Texture height.
		 */
		inline uint32_t getHeight() const
		{
			return m_height;
		}

		/**
		 * @brief Get texture image view.
		 * @return Texture image view.
		 */
		inline vk::ImageView& getImageView()
		{
			return m_imageView;
		}

		/**
		 * @brief Get texture sampler.
		 * @return Texture sampler.
		 */
		inline const vk::Sampler& getSampler() const
		{
			return m_sampler;
		}

		/**
		 * @brief Get texture filtering.
		 * @return Texture filtering.
		 */
		inline vk::Filter getFiltering() const
		{
			return m_filtering;
		}

	protected:

		/** Graphics context. */
		Graphics* m_graphics = nullptr;

		/** Filtering. */
		vk::Filter m_filtering = {};

		/** Texture image. */
		vk::Image m_image = {};

		/** Texture image view. */
		vk::ImageView m_imageView = {};

		/** Texture sampler. */
		vk::Sampler m_sampler = {};

		/** Texture image memory. */
		vk::DeviceMemory m_imageMemory = {};

		/** Texture width. */
		uint32_t m_width = 0;

		/** Texture height. */
		uint32_t m_height = 0;
	};



	/**
	 * @class Cubemap
	 * @brief 3D texture.
	 */
	class Cubemap : public Texture
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Image.
		 * @param Image view.
		 * @param Sampler.
		 * @param Image memory.
		 * @param Width.
		 * @param Height.
		 */
		Cubemap
		(
			Graphics* graphics, 
			vk::Image image, 
			vk::ImageView& imageView, 
			vk::Sampler sampler, 
			vk::DeviceMemory memory, 
			uint32_t width, 
			uint32_t height
		);

		/**
		 * @brief Constructor.
		 * @param Graphics context.
		 * @param Path to file containing top image.
		 * @param Path to file containing bottom image.
		 * @param Path to file containing north image.
		 * @param Path to file containing east image.
		 * @param Path to file containing south image.
		 * @param Path to file containing west image.
		 * @param Texture filtering.
		 */
		Cubemap
		(
			Graphics* graphics, 
			const std::string& top, 
			const std::string& bottom,
			const std::string& north,
			const std::string& east,
			const std::string& south,
			const std::string& west,
			vk::Filter filter
		);
	};
}