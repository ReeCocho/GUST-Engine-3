#pragma once

/**
 * @file Lights.hpp
 * @brief Lights header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <Math.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>
#include <Transform.hpp>

#include "Engine.hpp"

namespace gust
{
	/**
	 * @class Light
	 * @brief Light base class.
	 */
	class Light
	{
	public:

		/**
		 * @brief Set intensity.
		 * @param New intensity.
		 * @return New intensity.
		 */
		inline float setIntensity(float intensity)
		{
			m_intensity = intensity;
			return m_intensity;
		}

		/**
		 * @brief Set color.
		 * @param New color.
		 * @return New color.
		 */
		inline glm::vec3 setColor(glm::vec3 color)
		{
			m_color = color;
			return m_color;
		}

		/**
		 * @brief Get intensity.
		 * @return Intensity.
		 */
		inline float getIntensity() const
		{
			return m_intensity;
		}

		/**
		 * @brief Get color.
		 * @return Color.
		 */
		inline glm::vec3 getColor() const
		{
			return m_color;
		}

	protected:
		
		/** Light transform. */
		Handle<Transform> m_transform = Handle<Transform>::nullHandle();

	private:

		/** Intensity. */
		float m_intensity = 1.0f;

		/** Color. */
		glm::vec3 m_color = { 1, 1, 1 };
	};



	/**
	 * @class PointLight
	 * @brief Allows an entity to emit light from its center.
	 */
	class PointLight : public Component<PointLight>, public Light
	{
		friend class PointLightSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		PointLight() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		PointLight(Entity entity, Handle<PointLight> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~PointLight();

		/**
		 * @brief Set range.
		 * @parma New range.
		 * @return New range.
		 */
		inline float setRange(float range)
		{
			m_range = range;
			return m_range;
		}

		/**
		 * @brief Get range.
		 * @return Range.
		 */
		inline float getRange() const
		{
			return m_range;
		}

	private:

		/** Range. */
		float m_range = 8.0f;
	};

	/**
	 * @class PointLightSystem
	 * @brief Implementation for point lights.
	 */
	class PointLightSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		PointLightSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~PointLightSystem();

		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		void onBegin() override;

		/**
		 * @brief Called once per tick, after onLateTick(), and before rendering.
		 * @param Delta time.
		 */
		void onPreRender(float deltaTime) override;
	};



	/**
	 * @class DirectionalLight
	 * @brief Allows an entity to emit light in a direction.
	 */
	class DirectionalLight : public Component<DirectionalLight>, public Light
	{
		friend class DirectionalLightSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		DirectionalLight() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		DirectionalLight(Entity entity, Handle<DirectionalLight> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~DirectionalLight();
	};

	/**
	 * @class DirectionalLightSystem
	 * @brief Implementation for directional lights.
	 */
	class DirectionalLightSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		DirectionalLightSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~DirectionalLightSystem();

		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		void onBegin() override;

		/**
		 * @brief Called once per tick, after onLateTick(), and before rendering.
		 * @param Delta time.
		 */
		void onPreRender(float deltaTime) override;
	};



	/**
	 * @class SpotLight
	 * @brief Allows an entity to emit conical light from a position in a direction.
	 */
	class SpotLight : public Component<SpotLight>, public Light
	{
		friend class SpotLightSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		SpotLight() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		SpotLight(Entity entity, Handle<SpotLight> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~SpotLight();

		/**
		 * @brief Set range.
		 * @parma New range.
		 * @return New range.
		 */
		inline float setRange(float range)
		{
			m_range = range;
			return m_range;
		}

		/**
		 * @brief Set angle.
		 * @parma New angle.
		 * @return New angle.
		 */
		inline float setAngle(float angle)
		{
			m_angle = angle;
			return m_angle;
		}

		/**
		 * @brief Get range.
		 * @return Range.
		 */
		inline float getRange() const
		{
			return m_range;
		}

		/**
		 * @brief Get angle.
		 * @return Angle.
		 */
		inline float getAngle() const
		{
			return m_angle;
		}

	private:

		/** Range. */
		float m_range = 8.0f;

		/** Angle of propigation. */
		float m_angle = 30.0f;
	};

	/**
	 * @class DirectionalLightSystem
	 * @brief Implementation for directional lights.
	 */
	class SpotLightSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		SpotLightSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~SpotLightSystem();

		/**
		 * @brief Called when a component is added to the system.
		 * @param Component to act upon.
		 */
		void onBegin() override;

		/**
		 * @brief Called once per tick, after onLateTick(), and before rendering.
		 * @param Delta time.
		 */
		void onPreRender(float deltaTime) override;
	};
}