#pragma once

/**
 * @file Camera.hpp
 * @brief Camera header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include "Math.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Transform.hpp"

namespace gust
{
	/**
	 * @class Camera
	 * @brief Describes a viewpoint in space.
	 */
	class Camera : public Component<Camera>
	{
		friend class CameraSystem;

	public:

		/**
		 * @brief Default constructor.
		 */
		Camera() = default;

		/**
		 * @brief Constructor.
		 * @param Entity the component is attached to
		 * @param Component handle
		 */
		Camera(Entity entity, Handle<Camera> handle);

		/**
		 * @brief Destructor.
		 * @see Component::~Component
		 */
		~Camera();

		/**
		 * @brief Get field of view.
		 * @return Field of view.
		 */
		inline float getFieldOfView() const
		{
			return m_fieldOfView;
		}

		/**
		 * @brief Get near clipping plane.
		 * @return Near clipping plane.
		 */
		inline float getNearClippingPlane() const
		{
			return m_nearClippingPlane;
		}

		/**
		 * @brief Get far clipping plane.
		 * @return Far clipping plane.
		 */
		inline float getFarClippingPlane() const
		{
			return m_farClippingPlane;
		}

		/**
		 * @brief Get view matrix.
		 * @return View matrix.
		 */
		inline glm::mat4 getView() const
		{
			return m_view;
		}

		/**
		 * @brief Get projection matrix.
		 * @return Projection matrix.
		 */
		inline glm::mat4 getProjection() const
		{
			return m_projection;
		}

		/**
		 * @brief Set field of view.
		 * @param New field of view.
		 * @return New field of view.
		 */
		inline float setFieldOfView(float fov)
		{
			m_fieldOfView = fov;
			generateProjectionMatrix();
			return m_fieldOfView;
		}

		/**
		 * @brief Set near clipping plane.
		 * @param New near clipping plane.
		 * @return New near clipping plane.
		 */
		inline float setNearClippingPlane(float ncp)
		{
			m_nearClippingPlane = ncp;
			generateProjectionMatrix();
			return m_nearClippingPlane;
		}

		/**
		 * @brief Set far clipping plane.
		 * @param New far clipping plane.
		 * @return N far clipping plane.
		 */
		inline float setFarClippingPlane(float fcp)
		{
			m_farClippingPlane = fcp;
			generateProjectionMatrix();
			return m_farClippingPlane;
		}

		/**
		 * @brief Set the main camera.
		 * @param New main camera.
		 * @return New main camera.
		 */
		inline static Handle<Camera> setMainCamera(Handle<Camera> camera)
		{
			mainCamera = camera;
			Engine::get().renderer.setMainCamera(camera->m_virtualCamera);
			return mainCamera;
		}

		/**
		 * @brief Get the main camera.
		 * @return The main camera.
		 */
		inline static Handle<Camera> getMainCamera()
		{
			return mainCamera;
		}

	private:

		/**
		 * @brief Generate projection matrix.
		 */
		void generateProjectionMatrix();

		/**
		 * @brief Generate view matrix.
		 */
		void generateViewMatrix();



		/** The active main camera. */
		static Handle<Camera> mainCamera;

		/** Virtual camera create by the renderer. */
		Handle<VirtualCamera> m_virtualCamera = Handle<VirtualCamera>::nullHandle();

		/** Transform. */
		Handle<Transform> m_transform = Handle<Transform>::nullHandle();

		/** View matrix. */
		glm::mat4 m_view = {};

		/** Projection matrix. */
		glm::mat4 m_projection = {};

		/** Near clipping plane. */
		float m_nearClippingPlane = 0.03f;

		/** Far clipping plane. */
		float m_farClippingPlane = 100.0f;

		/** Field of view. */
		float m_fieldOfView = 80.0f;
	};



	/**
	 * @class CameraSystem
	 * @brief Implementation of the Camera class.
	 */
	class CameraSystem : public System
	{
	public:

		/**
		 * @brief Constructor.
		 * @param Scene the system is in.
		 */
		CameraSystem(Scene* scene);

		/**
		 * @brief Destructor.
		 */
		~CameraSystem();

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

		/**
		 * @brief Called when a component is removed from the system.
		 */
		void onEnd() override;
	};
}